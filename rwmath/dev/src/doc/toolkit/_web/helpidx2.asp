<%@LANGUAGE="PYTHON" %>

<!--#include file="bar.inc" -->

<%
from win32com.shell import shell
import win32com.client
from adoenums import *
import re

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

# Recordset used to store doc items from hhk file
hhkItems = None
# The target frame for doc pages
hhkItemsTargetFrame = ""

# ****************************************************************
# Save the hhkItems to Session
# ****************************************************************
def SaveHHKItems(hhkItems,hhkItemsTargetFrame):
    DebugPrint("SaveHHKItems:")
    DebugPrint("    hhkItemsTargetFrame="+hhkItemsTargetFrame)

    objHolder = [hhkItems]
    Session.SetValue("hhkItems", objHolder)
    Session.SetValue("hhkItemsTargetFrame", hhkItemsTargetFrame)

# ****************************************************************
# Load the hhkItems from Session
# ****************************************************************
def LoadHHKItems():
    objHolder = Session("hhkItems")
    hhkItems = objHolder[0]
    hhkItemsTargetFrame = Session("hhkItemsTargetFrame")

    return (hhkItems,hhkItemsTargetFrame)

# ****************************************************************
# Create an empty record set to store the doc items
# ****************************************************************
def CreateHHKItems():
    DebugPrint("CreatehhkItems")

    hhkItems = win32com.client.Dispatch("ADODB.Recordset")
    hhkItems.CursorLocation = adUseClient
    hhkItems.CursorType = adOpenStatic
    hhkItems.LockType = adLockBatchOptimistic

    #Add Fields
    hhkItems.Fields.Append("name", adVarChar, 255)
    hhkItems.Fields.Append("level", adInteger)
    hhkItems.Fields.Append("url", adVarChar, 255)

    #Create the Recordset
    hhkItems.Open()

    return hhkItems

# ****************************************************************
# Delete record set
# ****************************************************************
def DeleteHHKItems(hhkItems):
    DebugPrint("DeleteHHKItems")
    
    hhkItems.Close()
    hhkItems = None

# ****************************************************************
# Load HHK file into docItems recordset
# ****************************************************************
def LoadHHKFile (filename):
    DebugPrint("LoadHHKFile(%s)" % filename)

    #**The following opens a text stream
    ReadStream = Server.MapPath(filename)
    # ReadStream = filename
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
    docItems = CreateHHKItems()
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
                DebugPrint("Tag: " + tag)
                #evaluate tags:
                if (tag=="UL"):
                    if (objectOpen):    
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
                        docItems.Fields("level").value = currLevel
                        docItems.Fields("url").value = ""
                        DebugPrint("Opened object.")
                elif (tag=="PARAM"):
                    keysValues = GetKeysValues(tagcontent)  #get key-value pairs from markup as a dictionary object
                    if (keysValues["NAME"] == "NAME"):
                        DebugPrint ("Value = " + keysValues["VALUE"])
                        docItems.Fields("name").value = keysValues["VALUE"]
                    elif (keysValues["NAME"] == "LOCAL"):
                        docItems.Fields("url").value = keysValues["VALUE"]
                    elif (keysValues["NAME"] == "FRAMENAME"):
                        docItemsTargetFrame = keysValues["VALUE"]
    
    TextStream.close()
    TextStream = None
   
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
# Render the hhkItems as a tree
# ****************************************************************
def RenderTree(hhkItems):
    Response.Write ('<PRE>')
    hhkItems.MoveFirst()
    while (not hhkItems.EOF):
        i=hhkItems("level").value-1
        while(i):
            Response.Write ('<IMG SRC="blank.gif" ALIGN="top">')
            i = i-1
        if (hhkItems("url").value != ""):
            Response.Write ('<A HREF="%s">%s</A>' % (hhkItems("url").value, hhkItems("name").value))
        else:
            Response.Write ('%s' % (hhkItems("name").value))
        Response.Write ('<BR>')
        
        hhkItems.MoveNext()

# ****************************************************************
# Main
# ****************************************************************

if (Session("hhkItems")==None):
    (hhkItems, hhkItemsTargetFrame) = LoadHHKFile("index.hhk")
    SaveHHKItems(hhkItems, hhkItemsTargetFrame)
else:
    (hhkItems, hhkItemsTargetFrame) = LoadHHKItems()
# ****************************************************************
%>

<HTML>
<HEAD>
<%
if (hhkItemsTargetFrame!=""):
    Response.Write ('<BASE target="' + hhkItemsTargetFrame + '">')
%>
<link rel="stylesheet" href="navstyle.css">
</HEAD>
<BODY leftmargin=0 marginwidth="0" topmargin=0 marginheight="0" onload="SetToolBar(2);">
<%
RenderTree(hhkItems)
%>
</BODY>
</HTML>
