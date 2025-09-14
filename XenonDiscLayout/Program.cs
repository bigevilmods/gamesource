using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.IO;

namespace XenonDiscLayout
{
    enum NodeType
    {
        Directory,
        File,
        Reserved,
        Volume
    }

    enum DiskLayer
    {
        Layer0  = 0,
        Layer1  = 1
    }

    class DiskNode : IComparable<DiskNode>
    {
        public static long BlockSize = 2048;
        public static long EccBlocks = 8;

        private NodeType m_type;
        private string m_subdir;
        private string m_name;
        private long m_size;
        private bool m_movable;
        private long m_lba;
        private long m_blocks;
        private DiskLayer m_layer;
        private bool m_slowFile;

        public NodeType Type { get { return m_type; } }
        public string SubDir { get { return m_subdir; } }
        public string Name { get { return m_name; } }
        public long Size { get { return m_size; } }

        public bool SlowFile
        {
            get { return m_slowFile; }
            set { m_slowFile = value; }
        }

        public bool Named
        {
            get { return m_subdir != null && m_name != null; }
        }

        public DiskLayer Layer
        {
            get { return m_layer; }
            set { m_layer = value; }
        }

        public bool Movable
        {
            get { return m_movable; }
        }

        public long LBA
        {
            get { return m_lba; }
            set { m_lba = value; }
        }

        public long BlockCount
        {
            get { return m_blocks; }
            set { m_blocks = value; }
        }

        public DiskNode(string diskPath, string realPath)
        {
            m_movable = true;

            FileInfo info = new FileInfo(realPath);

            if (0 != (info.Attributes & FileAttributes.Directory))
            {
                m_type = NodeType.Directory;
                m_size = 0;
            }
            else
            {
                m_type = NodeType.File;
                m_size = info.Length;
            }

            m_blocks = GetBlockSize(m_size);
            SetSubDirAndName(diskPath);
        }

        private void SetSubDirAndName(string path)
        {
            StringBuilder subdir = new StringBuilder();
            String[] tokens = path.Split('\\');
            // Skip the first directory split (paths always begin with a backslash)
            for (int i = 1; i < tokens.Length - 1; ++i)
            {
                subdir.Append(@"\");
                subdir.Append(tokens[i]);
            }

            m_name = tokens[tokens.Length - 1];

            if (0 == subdir.Length && 0 == m_name.Length)
                m_subdir = "<ROOT>";
            else if (0 == subdir.Length)
                m_subdir = @"\";
            else
                m_subdir = subdir.ToString();
        }

        public DiskNode(DiskLayer layer, NodeType type, long lba, long blocks)
        {
            m_type = type;
            m_layer = layer;
            m_movable = false;
            m_lba = lba;
            m_blocks = blocks;
        }

        private static long GetBlockSize(long fileSize)
        {
            if (0 == fileSize)
                return 1;
            else
                return (fileSize + BlockSize) / BlockSize;
        }

        public int CompareTo(DiskNode other)
        {
            if (m_slowFile && !other.SlowFile)
                return 1;
            else if (!m_slowFile && other.SlowFile)
                return -1;
            else
                return 0 - m_blocks.CompareTo(other.m_blocks);
        }
    }

    class BlockAllocator
    {
        private DiskLayer m_layer;
        private long m_pos, m_max;
        private long m_direction;
        private readonly long m_fastestBlock;

        public readonly static long BlocksPerLayer = 1783936;

        public BlockAllocator(DiskLayer layer, long fastestBlock, long lastBlock)
        {
            m_layer = layer;
            m_fastestBlock = fastestBlock;
            m_pos = m_fastestBlock;
            m_max = lastBlock;

            if (fastestBlock > lastBlock)
                m_direction = -1;
            else
                m_direction = 1;
        }

        public bool CanAccomodate(long blocks)
        {
            return blocks <= Math.Abs(m_max - m_pos);
        }

        public long GetOffsetFromFastestRegion(DiskNode n)
        {
            if (!n.Movable && n.Layer != m_layer)
                return long.MaxValue;
            else
                return Math.Abs(m_fastestBlock - m_pos);
        }

        public DiskNode GetPreAlign(DiskNode n)
        {
            if (n.SlowFile)
                return null;

            if (m_direction > 0)
            {
                long extents = m_pos % DiskNode.EccBlocks;
                if (extents != 0)
                {
                    long gap = DiskNode.EccBlocks - extents;
                    DiskNode result = new DiskNode(m_layer, NodeType.Reserved, m_pos, gap);
                    m_pos += gap;
                    return result;
                }
                else
                {
                    return null;
                }
            }
            else
            {
                long extents = (m_pos - n.BlockCount) % DiskNode.EccBlocks;
                if (extents != 0)
                {
                    long gap = DiskNode.EccBlocks - extents;
                    DiskNode result = new DiskNode(m_layer, NodeType.Reserved, m_pos-gap, gap);
                    m_pos -= gap;
                    return result;
                }
                else
                {
                    return null;
                }
            }
        }

        public void Allocate(DiskNode n)
        {
            if (n.Movable)
            {
                n.Layer = m_layer;

                if (m_direction > 0)
                {
                    n.LBA = m_pos;
                }
                else
                {
                    n.LBA = m_pos - n.BlockCount;
                }

                m_pos += n.BlockCount * m_direction;
            }
            else
            {
                if (n.Layer != m_layer)
                    throw new ApplicationException("Node " + n.Name + " added to wrong layer");
            }
        }
    }

    class LayoutGenerator
    {
        private string m_basePath;
        private List<DiskNode> m_nodes = new List<DiskNode>();

        public LayoutGenerator(string basePath)
        {
            m_basePath = basePath;

            m_nodes.Add(new DiskNode(DiskLayer.Layer0, NodeType.Volume, 32, 2));
            m_nodes.Add(new DiskNode(DiskLayer.Layer0, NodeType.Reserved, 48, 4096));

            GlobFiles();
        }

        private void GlobFiles()
        {
            Dictionary<string, DiskNode> namedNodes = new Dictionary<string,DiskNode>();

            string[] files = Directory.GetFiles(m_basePath, "*", SearchOption.AllDirectories);
            foreach (string p in files)
            {
                string diskPath = p.Substring(m_basePath.Length);

                DiskNode n = new DiskNode(diskPath, p);
                ApplyGameProperties(n);
                m_nodes.Add(n);
                namedNodes.Add(diskPath, n);

                StringBuilder builder = new StringBuilder();
                string[] elements = diskPath.Split('\\');
                for(int i=0; i<elements.Length-1; ++i)
                {
                    if (elements[i].Length != 0)
                    {
                        builder.Append(@"\");
                        builder.Append(elements[i]);
                    }

                    if (!namedNodes.ContainsKey(builder.ToString()))
                    {
                        DiskNode parentDir = new DiskNode(builder.ToString(), m_basePath + builder.ToString());
                        ApplyGameProperties(parentDir);
                        m_nodes.Add(parentDir);
                        namedNodes.Add(builder.ToString(), parentDir);
                    }
                }
            }
        }

        private static readonly string[] slowPatterns = {
            @"streaming",
            @"ondemand",
            @"async\",
            @"movies\",
            @"output\",
            @"$systemupdate",
            @"scripts\",
            @".lua",
            @".cfg",
            @"database.dbmanifest"
        };

        private static void ApplyGameProperties(DiskNode n)
        {
            string lowerName = (n.SubDir + n.Name).ToLower();
            foreach (string p in slowPatterns)
            {
                if (lowerName.Contains(p))
                {
                    n.SlowFile = true;
                    break;
                }
            }

            if (NodeType.Directory == n.Type)
                n.SlowFile = true;
        }

        public void Generate()
        {
            BlockAllocator[] layerAllocators = new BlockAllocator[2];
            layerAllocators[(int)DiskLayer.Layer0] = MakeBlockAllocator(DiskLayer.Layer0);
            layerAllocators[(int)DiskLayer.Layer1] = MakeBlockAllocator(DiskLayer.Layer1);

            m_nodes.Sort();

            List<DiskNode> gapNodes = new List<DiskNode>();

            foreach (DiskNode n in m_nodes)
            {
                BlockAllocator allocator = GetBestAllocator(layerAllocators, n);
                if (null == allocator)
                    throw new ApplicationException("Couldn't fit file on disk: " + n.Name);
                DiskNode gap = allocator.GetPreAlign(n);
                if (null != gap)
                {
                    gapNodes.Add(gap);
                }
                allocator.Allocate(n);
            }

            m_nodes.AddRange(gapNodes);
        }

        private static BlockAllocator GetBestAllocator(BlockAllocator[] allocators, DiskNode n)
        {
            BlockAllocator bestSoFar = null;
            long bestOffset = long.MaxValue;
            foreach (BlockAllocator a in allocators)
            {
                if (!a.CanAccomodate(n.BlockCount))
                    continue;
                long offset = a.GetOffsetFromFastestRegion(n);
                if (offset < bestOffset || null == bestSoFar)
                {
                    bestOffset = offset;
                    bestSoFar = a;
                }
            }

            return bestSoFar;
        }

        private static BlockAllocator MakeBlockAllocator(DiskLayer layer)
        {
            long fastest, slowest;

            if (DiskLayer.Layer0 == layer)
            {
                // HACK: Exclude space for reserved nodes.
                fastest = BlockAllocator.BlocksPerLayer - 1;
                slowest = 48 + 4096;
            }
            else
            {
                fastest = BlockAllocator.BlocksPerLayer;
                slowest = 2 * BlockAllocator.BlocksPerLayer - 1;
            }
            return new BlockAllocator(layer, fastest, slowest);
        }

        public void Write(string outputFile)
        {
            XmlWriterSettings settings = new XmlWriterSettings();
            settings.Indent = true;
            settings.OmitXmlDeclaration = true;
            settings.Encoding = Encoding.Default;
            using (Stream outStream = File.Create(outputFile))
            using (XmlWriter w = XmlWriter.Create(outStream, settings))
            {
                WritePrologue(w);
                WriteLayer(w, DiskLayer.Layer0);
                WriteLayer(w, DiskLayer.Layer1);
                WriteEpilogue(w);
            }
        }

        private void WritePrologue(XmlWriter w)
        {
            w.WriteStartElement("XboxGameDiscLayout");
            w.WriteStartElement("Header");

            w.WriteStartElement("Version");
            w.WriteAttributeString("Type", "schema");
            w.WriteAttributeString("Major", "1");
            w.WriteAttributeString("Minor", "1");
            w.WriteEndElement();

            w.WriteStartElement("Version");
            w.WriteAttributeString("Type", "compat");
            w.WriteAttributeString("Major", "1");
            w.WriteAttributeString("Minor", "0");
            w.WriteEndElement();

            w.WriteStartElement("LastSave");
            w.WriteAttributeString("DateTime", "0");
            w.WriteStartElement("Tool");
            w.WriteAttributeString("ID", "GDEngine");
            w.WriteStartElement("Version");
            w.WriteAttributeString("Type", "app");
            w.WriteAttributeString("Major", "1");
            w.WriteAttributeString("Minor", "16");
            w.WriteAttributeString("Build", "6274");
            w.WriteAttributeString("QFE", "0");
            w.WriteEndElement(); // Version
            w.WriteEndElement(); // Tool
            w.WriteEndElement(); // LastSave

            w.WriteEndElement(); // header

            w.WriteStartElement("Sources");
            w.WriteStartElement("Source");
            w.WriteAttributeString("Id", "0");
            w.WriteAttributeString("PCSrcDirectory", m_basePath);
            w.WriteAttributeString("SearchPattern", "*.*");
            w.WriteAttributeString("XboxMediaDirectory", @"\");
            w.WriteAttributeString("Recurse", "Yes");
            w.WriteEndElement();
            w.WriteEndElement(); // sources
    
            w.WriteStartElement("Disc");
            w.WriteAttributeString("Id", "0");
            w.WriteAttributeString("Type", "0");
        }

        private void WriteEpilogue(XmlWriter w)
        {
            w.WriteStartElement("Files");
            w.WriteAttributeString("Location", "scratch");
            w.WriteEndElement(); // files
            w.WriteEndElement(); // disc
            w.WriteEndElement(); // xboxgamedisclayout
        }

        private void WriteLayer(XmlWriter w, DiskLayer l)
        {
            w.WriteStartElement("Files");
            if (DiskLayer.Layer0 == l)
                w.WriteAttributeString("Location", "layer0");
            else
                w.WriteAttributeString("Location", "layer1");
            w.WriteAttributeString("Sectors", BlockAllocator.BlocksPerLayer.ToString());

            foreach (DiskNode n in m_nodes)
            {
                if (n.Layer != l)
                    continue;
                w.WriteStartElement("Object");
                w.WriteAttributeString("Type", n.Type.ToString());
                if (n.Named)
                {
                    w.WriteAttributeString("Root", "0"); // What's this
                    w.WriteAttributeString("SubDirectory", n.SubDir);
                    w.WriteAttributeString("Name", n.Name);
                    w.WriteAttributeString("Size", n.Size.ToString());
                }
                w.WriteAttributeString("LBA", n.LBA.ToString());
                w.WriteAttributeString("Blocks", n.BlockCount.ToString());
                w.WriteEndElement();
            }

            w.WriteEndElement(); // Files
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            string inputDir = null;
            string outputFile = null;

            for (int i=0; i<args.Length; ++i)
            {
                string arg = args[i];
                string value = (i + 1) < args.Length ? args[i+1] : null;
                if ("-inputDir" == arg)
                {
                    inputDir = value;
                    ++i;
                }
                else if ("-outputFile" == arg)
                {
                    outputFile = value;
                    ++i;
                }
            }

            if (null == inputDir)
            {
                Console.Error.WriteLine("Error: No input directory specified");
                return;
            }

            if (null == outputFile)
            {
                Console.Error.WriteLine("Error: No output file specified");
                return;
            }

            try
            {
                LayoutGenerator g = new LayoutGenerator(inputDir);
                g.Generate();
                g.Write(outputFile);
            }
            catch (System.Exception e)
            {
                Console.Error.Write("Error: ");
                Console.Error.WriteLine(e.Message);
            }

        }

        private static void Usage()
        {
            Console.Error.WriteLine(@"Usage: XenonDiscLayout -inputDir path\to\data -outputFile path\to\foo.xgd");
        }
    }
}
