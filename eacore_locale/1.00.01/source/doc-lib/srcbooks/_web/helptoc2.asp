<%@LANGUAGE="PYTHON" %>

<SCRIPT Language="JavaScript">

/************************************/
/* Browser Detect                   */
/************************************/
var NS4 = (navigator.userAgent.indexOf("Netscape")>=0) ? 1 : 0;
var OP4 = (navigator.userAgent.indexOf("Opera")>=0) ? 1 : 0;
var MAC = (navigator.appVersion.indexOf("Mac") != -1);

/************************************/
/* Force page reload                */
/************************************/
function refresh(itemNumStr)
{
    currentPos = location.href.indexOf("#");
    if(currentPos>-1)
    {
        currentPosStr = location.href.substr(currentPos+1)
        if(currentPosStr==itemNumStr)
        {
            location.reload();
        }
    }
}

/************************************/
/* Open or Close a Folder           */
/************************************/
function openclose(itemNumStr)
{
    location.href = "helptoc2.asp?FLDR="+itemNumStr+"#"+itemNumStr;
    if (NS4) refresh(itemNumStr);
}
</SCRIPT>

<!--#include file="bar.inc" -->

<%
# ****************************************************************
from win32com.shell import shell
import win32com.client
from adoenums import *
import re
import os

DebugFlag = 0
WebFlag = 1

# ****************************************************************
def DebugPrint (str):
    if (DebugFlag):
        Print (str)
# ****************************************************************
def Print (str):
    if (WebFlag):
        Response.Write ("<FONT COLOR='#CC0000'>" + str + "</FONT><BR>")
    else:
        print (str)

# ****************************************************************
# Definitions
# ****************************************************************

# Enums for hhcItems.Fields("itemType")
TYPE_FOLDER  = 0
TYPE_FILE    = 1

# Enums for hhcItems.Fields("imageNumber")
IMAGE_DEFAULT = 0

# Recordset used to store doc items from HHC file
hhcItems = None
# The target frame for doc pages
hhcItemsTargetFrame = ""

# item # of folder user clicked on
reqFldrID = 0

# ****************************************************************
# Save the hhcItems to Session
# ****************************************************************
def SaveHHCItems(hhcItems,hhcItemsTargetFrame):
    DebugPrint("SaveHHCItems:")
    DebugPrint("    hhcItemsTargetFrame="+hhcItemsTargetFrame)

    objHolder = [hhcItems]
    Session.SetValue("hhcItems", objHolder)
    Session.SetValue("hhcItemsTargetFrame", hhcItemsTargetFrame)

# ****************************************************************
# Load the hhcItems from Session
# ****************************************************************
def LoadHHCItems(reqFldrID):
    DebugPrint("LoadHHCItems")

    objHolder = Session("hhcItems")
    hhcItems = objHolder[0]
    hhcItemsTargetFrame = Session("hhcItemsTargetFrame")

    if (reqFldrID >= 0):
        hhcItems.AbsolutePosition = reqFldrID
        if (hhcItems.Fields("folderOpen").value == 0):
            hhcItems.Fields("folderOpen").value = 1
        else:
            hhcItems.Fields("folderOpen").value = 0
        hhcItems.Update()

    return (hhcItems,hhcItemsTargetFrame)

# ****************************************************************
# Create an empty record set to store the doc items
# ****************************************************************
def CreateHHCItems():
    DebugPrint("CreatehhcItems")

    hhcItems = win32com.client.Dispatch("ADODB.Recordset")
    hhcItems.CursorLocation = adUseClient
    hhcItems.CursorType = adOpenStatic
    hhcItems.LockType = adLockBatchOptimistic

    #Add Fields
    hhcItems.Fields.Append("itemType", adInteger)
    hhcItems.Fields.Append("name", adVarChar, 255)
    hhcItems.Fields.Append("level", adInteger)
    hhcItems.Fields.Append("url", adVarChar, 255)
    hhcItems.Fields.Append("imageNumber", adInteger)
    hhcItems.Fields.Append("folderOpen", adInteger)
    hhcItems.Fields.Append("nextSibling", adInteger)

    #Create the Recordset
    hhcItems.Open()

    return hhcItems

# ****************************************************************
# Delete record set
# ****************************************************************
def DeleteHHCItems(hhcItems):
    DebugPrint("DeleteHHCItems")
    
    hhcItems.Close()
    hhcItems = None

# ****************************************************************
# Load HHC file into docItems recordset
# ****************************************************************
def LoadHHCFile (filename):
    DebugPrint("LoadHHCFile(%s)" % filename)

    #**The following opens a text stream
    if (WebFlag):
        ReadStream = Server.MapPath(filename)
    else:
        os.chdir(os.getcwd())
        ReadStream = filename
    TextStream = open(ReadStream,"r")
    	
    #** Check for failed file open
    if (not TextStream):
        if (not DebugFlag):
            Response.Write ("<b>ERROR:</b> Status file \"" + filename + "\" does not exist!<br>")
        else:
            print ("<b>ERROR:</b> Status file \"" + filename + "\" does not exist!<br>")
    	return (None,None)

    #parse file and put doc items into record set
    currLevel = 0
    inDocItem = 0
    objectOpen = 0
    docItems = CreateHHCItems()
    docItemsTargetFrame = ""
    line = "\n"
    while (line != ""):
        line = TextStream.readline().strip()		# read a line
        str = line
        while (str != ""):              # parse line
            DebugPrint("Line: " + str)
            matches = re.match(r'<(?P<tag>[^ >]+)\s*(?P<tagcontent>([^>"]*"[^"]*?"[^>"]*)*)>(?P<remainder>.*)', str)
            if (matches):
                tag = matches.group("tag").upper()
                tagcontent = matches.group("tagcontent")
                str = matches.group("remainder")
                DebugPrint("tag: " + tag)
                DebugPrint("tagcontent: " + tagcontent)
                DebugPrint("str: " + str)
                #evaluate tags:
                if (tag=="UL"):
                    if (objectOpen):    
                        docItems.Fields("itemType").value = TYPE_FOLDER   #last object was a folder
                        # save object:
                        docItems.Update()
                        docItems.MoveLast()
                        objectOpen = 0
                        DebugPrint("Closed object.")
                    currLevel = currLevel + 1
                elif (tag=="/UL"):
                    currLevel = currLevel - 1
                    if (objectOpen):    
                        # save object:
                        docItems.Update()
                        docItems.MoveLast()
                        objectOpen = 0
                        DebugPrint("Closed object.")
                elif (tag=="LI"):
                    if (objectOpen):    
                        # save object:
                        docItems.Update()
                        docItems.MoveLast()
                        objectOpen = 0
                        DebugPrint("Closed object.")
                elif (tag=="OBJECT"):
                    if (currLevel>0):
                        # new object to be added to TOC (assume type="text/sitemap"):
                        objectOpen = 1
                        docItems.AddNew()
                        docItems.Fields("itemType").value = TYPE_FILE
                        docItems.Fields("level").value = currLevel
                        docItems.Fields("imageNumber").value = IMAGE_DEFAULT
                        docItems.Fields("folderOpen").value = 0
                        docItems.Fields("nextSibling").value = 0
                        docItems.Fields("url").value = ""
                        DebugPrint("Opened object.")
                elif (tag=="PARAM"):
                    keysValues = GetKeysValues(tagcontent)  #get key-value pairs from markup as a dictionary object
                    if (keysValues["NAME"] == "NAME"):
                        DebugPrint ("Value = " + keysValues["VALUE"])
                        docItems.Fields("name").value = keysValues["VALUE"]
                    elif (keysValues["NAME"] == "LOCAL"):
                        docItems.Fields("url").value = keysValues["VALUE"]
                    elif (keysValues["NAME"] == "IMAGENUMBER"):
                        docItems.Fields("imageNumber").value = int(keysValues["VALUE"])
                    elif (keysValues["NAME"] == "FRAMENAME"):
                        docItemsTargetFrame = keysValues["VALUE"]
    
    TextStream.close()
    TextStream = None
   
    docItems = SetNextSiblings(docItems)
    
    #DebugPrint("docItems.RecordCount="+str(docItems.RecordCount))
    return (docItems, docItemsTargetFrame)

# ****************************************************************
# Extract key-values pairs from an HTML tag into a dictionary.
# ****************************************************************
def GetKeysValues(tagcontent):
    d = {}
    while (tagcontent != ""):
        matches = re.match(r'\s*(?P<key>\w+)\s*=\s*\"(?P<value>.*?)\"(?P<remainder>.*)', tagcontent)
        if (matches):
            key = matches.group("key").upper()
            if (key=="NAME"):
                d[key] = matches.group("value").upper()
            else:
                d[key] = matches.group("value")
            tagcontent = matches.group("remainder")
        else:
            tagcontent = ""

    return d    

# ****************************************************************
# Iterates through all records and for each, sets the 'nextSibling'
# field to the absolute position of the next record at the same level.
# ****************************************************************
def SetNextSiblings (docItems):
    DebugPrint ("SetNextSiblings:")
    docItems.MoveFirst()
    while (not docItems.EOF):
        if (docItems.Fields("itemType").value==TYPE_FOLDER):
            DebugPrint ("    Folder at: %d" % (docItems.AbsolutePosition))
            docItems.Fields("nextSibling").value = GetNextSibling(docItems)
            DebugPrint ("    Next sibling: %d" % (docItems.Fields("nextSibling").value))
            docItems.Update()
        docItems.MoveNext()

    return docItems   

# ****************************************************************
# Finds the position of the 'nextSibling' to current item.
# 0 if no next sibling.
# ****************************************************************
def GetNextSibling (docItems):
    matchRecordPos = docItems.AbsolutePosition
    matchLevel = docItems.Fields("level").value
    nextSiblingFound = 0
    while ((not nextSiblingFound) and (not docItems.EOF)):
        docItems.MoveNext()
        if (not docItems.EOF):
            if (docItems.Fields("level").value <= matchLevel):
                nextSiblingFound = 1
    if (nextSiblingFound):
        GetNextSibling = docItems.AbsolutePosition
    else:
        GetNextSibling = docItems.RecordCount+1
    docItems.AbsolutePosition = matchRecordPos

    return GetNextSibling    

# ****************************************************************
# Render the hhcItems as a tree
# ****************************************************************
def RenderTree(hhcItems):
    DebugPrint("RenderTree")

    Response.Write ("<PRE>")
    hhcItems.MoveFirst()
    while (not hhcItems.EOF):
        if (int(hhcItems("itemType")) == TYPE_FOLDER):
            DebugPrint('hhcItems("level")=' + str(hhcItems("level")))
            DebugPrint(str(hhcItems.AbsolutePosition) + ':' + str(hhcItems("nextSibling")) + ':')
            Response.Write ('<a name="' + str(hhcItems.AbsolutePosition) + '"></a>')
            i=hhcItems("level").value-1
            while(i):
                Response.Write ('<IMG SRC="blank.gif" ALIGN="top">')
                i = i-1
            if (hhcItems("folderOpen").value):
                nodeImage = "mnode.gif"
                if (hhcItems("imageNumber").value == IMAGE_DEFAULT):
                    fldrImage = "2.gif"
                else:
                    fldrImage = str(hhcItems("imageNumber").value) + ".gif"
            else:       # folderOpen == 0
                nodeImage = "pnode.gif"
                if (hhcItems("imageNumber").value == IMAGE_DEFAULT):
                    fldrImage = "1.gif"
                else:
                    fldrImage = str(hhcItems("imageNumber").value) + ".gif"
            imageNumber = hhcItems("imageNumber").value
            Response.Write ('<A HREF="JavaScript:openclose(\'' + str(hhcItems.AbsolutePosition) + '\')" TARGET="_self" TITLE="Click to Open/Close">')
            Response.Write ('<IMG SRC="' + nodeImage + '" ALIGN="top" BORDER="0">')
            Response.Write ('<IMG SRC="' + fldrImage + '" ALIGN="top" BORDER="0">')
            Response.Write ('</A>')
            if (hhcItems("url").value!=""):
                Response.Write ('<A HREF="%s">%s</A>' % ( hhcItems("url").value, hhcItems("name").value ) )
            else:
                Response.Write (hhcItems("name"))
            Response.Write ('<BR>')
            
            # Skip items if folder is closed
            if (hhcItems("folderOpen").value==0):
                DebugPrint("folder closed")
                if (hhcItems("nextSibling").value>0):
                    hhcItems.AbsolutePosition = hhcItems("nextSibling").value-1
                    DebugPrint("Skipping to " + str(hhcItems.AbsolutePosition))
        else:    # itemType = TYPE_FILE
            i=hhcItems("level").value
            while(i):
                Response.Write ('<IMG SRC="blank.gif" ALIGN="top">')
                i = i-1
            imageNumber = hhcItems("imageNumber").value
            if (imageNumber==IMAGE_DEFAULT):
                imageNumber = 9
            if (hhcItems("url").value != ""):
                Response.Write ('<A HREF="' + hhcItems("url").value + '">')
            Response.Write ('<IMG SRC="' + str(imageNumber) + '.gif" ALIGN="top" BORDER="0">')
            Response.Write (hhcItems("name").value)
            if (hhcItems("url").value != ""):
                Response.Write ('</A>')
            Response.Write ('<BR>')
        hhcItems.MoveNext()

# ****************************************************************
# Main
# ****************************************************************

queryString = {}
if (WebFlag):
    for key in Request.QueryString:
        queryString[key] = Request.QueryString(key)

if (queryString.has_key("FLDR")):
    reqFldrID = int(queryString["FLDR"])
else:
    reqFldrID = -1
DebugPrint ("reqFldrID="+str(reqFldrID))

# DEBUG
#(hhcItems, hhcItemsTargetFrame) = LoadHHCFile("index.hhc")
if (WebFlag):
    if (Session("hhcItems")==None):
        DebugPrint("LoadHHCFile")
        (hhcItems, hhcItemsTargetFrame) = LoadHHCFile("index.hhc")
        SaveHHCItems(hhcItems, hhcItemsTargetFrame)
    else:
        (hhcItems, hhcItemsTargetFrame) = LoadHHCItems(reqFldrID)
else:
    (hhcItems, hhcItemsTargetFrame) = LoadHHCFile("index.hhc")

# ****************************************************************
%>

<HTML>
<HEAD>
<%
if (hhcItemsTargetFrame!=""):
    Response.Write ('<BASE target="' + hhcItemsTargetFrame + '">')
%>
<link rel="stylesheet" href="navstyle.css">
</HEAD>
<BODY leftmargin=0 marginwidth="0" topmargin=0 marginheight="0" onload="SetToolBar(1);">
<%
RenderTree(hhcItems)
%>
</BODY>
</HTML>
