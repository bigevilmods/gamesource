<%

' ----------------------------------------------------------------
' Load HHC file into docItems recordset
' ----------------------------------------------------------------
FUNCTION LoadHHCFile (fileName, ByRef docItems, ByRef docItemsTargetFrame)
    DIM ReadStream,FileStream,TextStream
    DIM str,markup
    DIM tagValueDict
    DIM currLevel
    DIM inOBJECT
    DIM lastItem
    DIM paramName,paramValue

    'DebugPrint("LoadHHCFile(" & fileName & ")")

    '**The following opens a text stream
    ReadStream = Cstr(Server.MapPath(filename)) 
    SET FileStream = CreateObject("Scripting.FileSystemObject")
    SET TextStream = FileStream.OpenTextFile (ReadStream)
    	
    '** Check for failed file open
    IF IsNull(TextStream) THEN
    	Response.Write "<b>ERROR:</b> Status file """ & filename & """ does not exist!<br>"
    	EXIT FUNCTION
    END IF

    'parse file and put doc items into record set
    currLevel = 0
    inDocItem = FALSE
    WHILE NOT TextStream.AtEndOfStream
        str = TextStream.ReadLine		'read a line
        DO
            markup = GetMarkup(str)            'get next tag from line
            IF (markup<>"") THEN
                ''DebugPrint("markup=" & Server.HTMLEncode(markup))
                SET tagValueDict = GetTagValues(markup) 'get tag-value pairs from markup as a dictionary object
                tags = tagValueDict.Keys                'get array of keys
                FOR i = 0 TO tagValueDict.Count-1       'iterate the array
                    tag = UCase(tags(i))
                    value = tagValueDict.Item(tags(i))  'get item matching key
                    ''DebugPrint(Server.HTMLEncode(tag) & "=" & Server.HTMLEncode(value))
                    'evaluate tag/values
                    SELECT CASE tag
                        CASE "UL"
                            currLevel = currLevel + 1
                            IF (docItems.RecordCount>0) THEN    'last object was a folder
                                'DebugPrint("<B>" & docItems.Fields("name") & "</B> - is now TYPE_FOLDER")
                                docItems.Fields("itemType") = TYPE_FOLDER
                                docItems.Update
                                docItems.MoveLast
                            END IF
                        CASE "/UL"
                            currLevel = currLevel - 1
                        CASE "LI"
                        CASE "OBJECT"
                            inOBJECT = TRUE
                            IF (currLevel>0) THEN
                                docItems.AddNew
                                docItems.Fields("itemType") = TYPE_FILE
                                docItems.Fields("level") = currLevel
                                docItems.Fields("folderOpen") = FALSE
                                docItems.Fields("nextSibling") = 0
                            END IF
                        CASE "NAME"
                            SELECT CASE value
                                CASE "Name"         paramName = "name"
                                CASE "Local"        paramName = "url"
                                CASE "ImageNumber"  paramName = "imageNumber"
                                CASE "FrameName"    paramName = "frameName"
                                CASE ELSE           paramName = ""
                            END SELECT
                        CASE "VALUE"
                            IF (inOBJECT) THEN
                                IF (currLevel>0) THEN
                                    'DebugPrint("paramName="&paramName)
                                    docItems.Fields(paramName) = value
                                ELSEIF (paramName="frameName") THEN
                                    docItemsTargetFrame = value
                                ELSE
                                END IF
                            END IF
                        CASE "/OBJECT"
                            inOBJECT = FALSE
                            IF (currLevel>0) THEN
                                docItems.Update
                            END IF
                            IF (inSITEPROPERTIES) THEN
                                inSITEPROPERTIES = FALSE
                            END IF
                        CASE "TYPE"
                            IF (value="text/site properties") THEN
                                inSITEPROPERTIES = TRUE
                            END IF
                            
                    END SELECT
                NEXT
            END IF
        LOOP UNTIL (markup="")
    WEND
    
    TextStream.Close
    SET TextStream = NOTHING
    SET FileStream = NOTHING
   
    SetNextSiblings(docItems)
    
    'DebugPrint("docItems.RecordCount="&docItems.RecordCount)
END FUNCTION

' ----------------------------------------------------------------
' Load HHK file into docItems recordset
' ----------------------------------------------------------------
FUNCTION LoadHHKFile (fileName, ByRef docItems, ByRef docItemsTargetFrame)
    DIM ReadStream,FileStream,TextStream
    DIM str,markup
    DIM tagValueDict
    DIM currLevel
    DIM inOBJECT
    DIM lastItem
    DIM paramName,paramValue

    'DebugPrint("LoadHHCFile(" & fileName & ")")

    '**The following opens a text stream
    ReadStream = Cstr(Server.MapPath(filename)) 
    SET FileStream = CreateObject("Scripting.FileSystemObject")
    SET TextStream = FileStream.OpenTextFile (ReadStream)
    	
    '** Check for failed file open
    IF IsNull(TextStream) THEN
    	Response.Write "<b>ERROR:</b> Status file """ & filename & """ does not exist!<br>"
    	EXIT FUNCTION
    END IF

    'parse file and put doc items into record set
    currLevel = 0
    inDocItem = FALSE
    WHILE NOT TextStream.AtEndOfStream
        str = TextStream.ReadLine		'read a line
        DO
            markup = GetMarkup(str)            'get next tag from line
            IF (markup<>"") THEN
                ''DebugPrint("markup=" & Server.HTMLEncode(markup))
                SET tagValueDict = GetTagValues(markup) 'get tag-value pairs from markup as a dictionary object
                tags = tagValueDict.Keys                'get array of keys
                FOR i = 0 TO tagValueDict.Count-1       'iterate the array
                    tag = UCase(tags(i))
                    value = tagValueDict.Item(tags(i))  'get item matching key
                    valuelcase = LCase(value)
                    'DebugPrint(Server.HTMLEncode(tag) & "=" & Server.HTMLEncode(value))
                    'evaluate tag/values
                    SELECT CASE tag
                        CASE "UL"
                            currLevel = currLevel + 1
                        CASE "/UL"
                            currLevel = currLevel - 1
                        CASE "LI"
                        CASE "OBJECT"
                            inOBJECT = TRUE
                            IF (currLevel>0) THEN
                                docItems.AddNew
                                docItems.Fields("level") = currLevel
                            END IF
                        CASE "NAME"
                            SELECT CASE valuelcase
                                CASE "name"         paramName = "name"
                                CASE "local"        paramName = "url"
                                CASE "framename"    paramName = "frameName"
                                CASE ELSE           paramName = ""
                            END SELECT
                        CASE "VALUE"
                            IF (inOBJECT) THEN
                                IF (currLevel>0 AND paramName<>"") THEN
                                    docItems.Fields(paramName) = value
                                ELSEIF (paramName="frameName") THEN
                                    docItemsTargetFrame = value
                                ELSE
                                END IF
                            END IF
                        CASE "/OBJECT"
                            inOBJECT = FALSE
                            IF (currLevel>0) THEN
                                docItems.Update
                            END IF
                            IF (inSITEPROPERTIES) THEN
                                inSITEPROPERTIES = FALSE
                            END IF
                        CASE "TYPE"
                            IF (value="text/site properties") THEN
                                inSITEPROPERTIES = TRUE
                            END IF
                            
                    END SELECT
                NEXT
            END IF
        LOOP UNTIL (markup="")
    WEND
    
    TextStream.Close
    SET TextStream = NOTHING
    SET FileStream = NOTHING
   
    'DebugPrint("docItems.RecordCount="&docItems.RecordCount)
END FUNCTION

' ----------------------------------------------------------------
' Get first <> markup
' ----------------------------------------------------------------
FUNCTION GetMarkup(ByRef str)
    DIM startPos, endPos

    IF (str<>"") THEN
        startPos = InStr(1,str,"<",1)   'find tag start
        endPos   = InStr(1,str,">",1)   'find tag end
        IF (startPos>0 AND endPos>0) THEN
            GetMarkup = MID(str,startPos,endPos-startPos+1)
            str = MID(str,endPos+1)
        ELSE
            GetMarkup = ""
        END IF
    ELSE
        GetMarkup = ""
    END IF
END FUNCTION

' ----------------------------------------------------------------
' Get tag-value pairs from markup
' ----------------------------------------------------------------
FUNCTION GetTagValues(markup)
    DIM d
    DIM tagStr,valueStr,char
    DIM tagStart,tagEnd,valStart,valEnd
    DIM i
    
    SET d = Server.CreateObject("Scripting.Dictionary")
    markup = Mid(markup,2)  'skip opening '<'
    DO
        'get tag:
        markup = LTrim(markup)      'skip spaces
        tagStart = 1
        markupEnd   = InStr(1,markup,">",1)
        separator   = InStr(1,markup," ",1)
        equal       = InStr(1,markup,"=",1)
        IF (separator=0 AND equal=0) THEN
            tagEnd = markupEnd-1
        ELSEIF (separator>0 AND equal>0) THEN
            IF (separator<equal) THEN
                tagEnd = separator-1
            ELSE    '(equal<separator)
                tagEnd = equal-1
            END IF
        ELSEIF (separator>0) THEN
            tagEnd = separator-1
        ELSE    '(equal>0)
            tagEnd = equal-1
        END IF
        tagStr = Mid(markup,tagStart,tagEnd-tagStart+1)
        markup = Mid(markup,tagEnd+1)

        'get value:
        markup = LTrim(markup)          'skip spaces
        IF (Mid(markup,1,1)="=") THEN
            markup = LTrim(Mid(markup,2))   'skip '='
            tagStart = 1
            IF (Mid(markup,1,1)="""") THEN      'string value
                markup = LTrim(Mid(markup,2))   'skip opening "
                tagEnd = InStr(1,markup,"""",1) - 1
                valueStr = Mid(markup,tagStart,tagEnd-tagStart+1)
                markup = Mid(markup,tagEnd+2)
            ELSE
                markupEnd   = InStr(1,markup,">",1)
                separator   = InStr(1,markup," ",1)
                equal       = InStr(1,markup,"=",1)
                IF (separator=0 AND equal=0) THEN
                    tagEnd = markupEnd-1
                ELSEIF (separator>0 AND equal>0) THEN
                    IF (separator<equal) THEN
                        tagEnd = separator-1
                    ELSE    '(equal<separator)
                        tagEnd = equal-1
                    END IF
                ELSEIF (separator>0) THEN
                    tagEnd = separator-1
                ELSE    '(equal>0)
                    tagEnd = equal-1
                END IF
                valueStr = Mid(markup,tagStart,tagEnd-tagStart+1)
                markup = Mid(markup,tagEnd+1)
            END IF
        ELSE
            valueStr = ""
        END IF
        
        d.Add tagStr,valueStr
    LOOP UNTIL (markup=">")
    
    SET GetTagValues = d
END FUNCTION

' ----------------------------------------------------------------
' Is the character an alphanumeric?
' ----------------------------------------------------------------
FUNCTION IsAlphaNum(char)
    IF ((char>="A" AND char<="Z") OR (char>="a" AND char<="z") OR (char>="0" AND char<="9")) THEN
        IsAlphaNum = TRUE
    ELSE
        IsAlphaNum = FALSE
    END IF
END FUNCTION

' ----------------------------------------------------------------
' Iterates through all records and for each, sets the 'nextSibling'
' field to the absolute position of the next record at the same level.
' ----------------------------------------------------------------
FUNCTION SetNextSiblings (ByRef docItems)
    DIM lastFldrOfLevel(32)
    DIM currRecordPos, lastRecordPos
    
    docItems.MoveFirst
    WHILE (NOT docItems.EOF)
        IF (docItems("itemType")=FOLDER_TYPE) THEN
            docItems("nextSibling") = GetNextSibling(docItems)
            docItems.Update
        END IF
        docItems.MoveNext
    WEND
END FUNCTION

' ----------------------------------------------------------------
' Finds the position of the 'nextSibling' to current item.
' 0 if no next sibling.
' ----------------------------------------------------------------
FUNCTION GetNextSibling (ByRef docItems)
    matchRecordPos = docItems.AbsolutePosition
    matchLevel = docItems("level")
    nextSiblingFound = FALSE
    WHILE ((NOT nextSiblingFound) AND (NOT docItems.EOF))
        docItems.MoveNext
        IF (NOT docItems.EOF) THEN
            IF (docItems("level") <= matchLevel) THEN
                nextSiblingFound = TRUE
            END IF
        END IF
    WEND
    IF (nextSiblingFound) THEN
        GetNextSibling = docItems.AbsolutePosition
    ELSE
        GetNextSibling = docItems.RecordCount+1
    END IF
    docItems.AbsolutePosition = matchRecordPos
END FUNCTION

%>
