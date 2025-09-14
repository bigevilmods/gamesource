
<%
' enums

' CursorLocationEnum
CONST adUseNone = 1
CONST adUseClient = 2
CONST adUseServer = 3

' CursorTypeEnum
CONST adOpenUnspecified = -1    'Does not specify the type of cursor. 
CONST adOpenForwardOnly = 0     'Default. Uses a forward-only cursor. Identical to a static cursor, except that you can only scroll forward through records. This improves performance in situations when you need to make only a single pass through a Recordset. 
CONST adOpenKeyset = 1          'Uses a keyset cursor. Like a dynamic cursor, except that you can't see records that other users add, although records that other users delete are inaccessible from your recordset. Data changes by other users are still visible. 
CONST adOpenDynamic = 2         'Uses a dynamic cursor. Additions, changes, and deletions by other users are visible, and all types of movement through the recordset are allowed, except for bookmarks, if the provider doesn't support them. 
CONST adOpenStatic = 3          'Uses a static cursor. A static copy of a set of records that you can use to find data or generate reports. Additions, changes, or deletions by other users are not visible. 

' LockTypeEnum
CONST adLockUnspecified = -1    'Does not specify a type of lock. For clones, the clone is created with the same lock type as the original. 
CONST adLockReadOnly = 1        'Indicates read-only—you cannot alter the data. 
CONST adLockPessimistic = 2     'Indicates pessimistic locking, record by record—the provider does what is necessary to ensure successful editing of the records, usually by locking records at the data source immediately upon editing. 
CONST adLockOptimistic = 3      'Indicates optimistic locking, record by record—the provider uses optimistic locking, locking records only when you call the Update method. 
CONST adLockBatchOptimistic = 4 'Indicates optimistic batch updates—required for batch update mode as opposed to immediate update mode. 

' FieldAttributeEnum
CONST adFldUnspecified = -1
CONST adFldMayDefer = &H2 
CONST adFldUpdatable = &H4 
CONST adFldUnknownUpdatable = &H8 
CONST adFldFixed = &H10 
CONST adFldIsNullable = &H20 
CONST adFldMayBeNull = &H40 
CONST adFldLong = &H80 
CONST adFldRowID = &H100 
CONST adFldRowVersion = &H200 
CONST adFldCacheDeferred = &H1000 
CONST adFldNegativeScale = &H4000 
CONST adFldKeyColumn = &H8000 

' DataTypeEnum
CONST adBigInt = 20         'Indicates an eight-byte signed integer (DBTYPE_I8). 
CONST adBinary = 128        'Indicates a binary value (DBTYPE_BYTES). 
CONST adBoolean = 11        'Indicates a Boolean value (DBTYPE_BOOL). 
CONST adBSTR = 8            'Indicates a null-terminated character string (Unicode) (DBTYPE_BSTR). 
CONST adChapter = 136       'Indicates a four-byte chapter value used to identify rows in a child rowset (DBTYPE_HCHAPTER). 
CONST adChar = 129          'Indicates a String value (DBTYPE_STR). 
CONST adCurrency = 6        'Indicates a currency value (DBTYPE_CY). Currency is a fixed-point number with four digits to the right of the decimal point. It is stored in an eight-byte signed integer scaled by 10,000. 
CONST adDate = 7            'Indicates a date value (DBTYPE_DATE). A date is stored as a Double, the whole part of which is the number of days since December 30, 1899, and the fractional part of which is the fraction of a day. 
CONST adDBDate = 133        'Indicates a date value (yyyymmdd) (DBTYPE_DBDATE). 
CONST adDBTime = 134        'Indicates a time value (hhmmss) (DBTYPE_DBTIME). 
CONST adDBTimeStamp = 135   'Indicates a date-time stamp (yyyymmddhhmmss plus a fraction in billionths) (DBTYPE_DBTIMESTAMP). 
CONST adDecimal = 14        'Indicates aAn exact numeric value with a fixed precision and scale (DBTYPE_DECIMAL). 
CONST adDouble = 5          'Indicates a double-precision floating- point value (DBTYPE_R8). 
CONST adEmpty = 0           'Specifies nNo value was specified (DBTYPE_EMPTY). 
CONST adError = 10          'Indicates a 32-bit error code (DBTYPE_ERROR). 
CONST adFileTime = 64       'Indicates a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (DBTYPE_FILETIME). 
CONST adGUID = 72           'Indicates a globally unique identifier (GUID) (DBTYPE_GUID). 
CONST adIDispatch = 9       'Indicates a pointer to an IDispatch interface on an OLE object (DBTYPE_IDISPATCH). 
CONST adInteger = 3         'Indicates a four4-byte signed integer (DBTYPE_I4). 
CONST adIUnknown = 13       'Indicates a pointer to an IUnknown interface on an OLE object (DBTYPE_IUNKNOWN). 
CONST adLongVarBinary = 205 'Indicates a long binary value (Parameter object only). 
CONST adLongVarChar = 201   'Indicates a long String value (Parameter object only). 
CONST adLongVarWChar = 203  'Indicates a long null-terminated string value (Parameter object only). 
CONST adNumeric = 131       'Indicates aAn exact numeric value with a fixed precision and scale (DBTYPE_NUMERIC). 
CONST adPropVariant = 138   'Indicates aAn Automation PROPVARIANT (DBTYPE_PROP_VARIANT). 
CONST adSingle = 4          'Indicates a single-precision floating- point value (DBTYPE_R4). 
CONST adSmallInt = 2        'Indicates a 2two-byte signed integer (DBTYPE_I2). 
CONST adTinyInt = 16        'Indicates a one1-byte signed integer (DBTYPE_I1). 
CONST adUnsignedBigInt = 21     'Indicates aAn eight8-byte unsigned integer (DBTYPE_UI8). 
CONST adUnsignedInt = 19        'Indicates a four4-byte unsigned integer (DBTYPE_UI4). 
CONST adUnsignedSmallInt = 18   'Indicates a two2-byte unsigned integer (DBTYPE_UI2). 
CONST adUnsignedTinyInt = 17    'Indicates a one1-byte unsigned integer (DBTYPE_UI1). 
CONST adUserDefined = 132       'Indicates a user-defined variable (DBTYPE_UDT). 
CONST adVarBinary = 204         'Indicates a binary value (Parameter object only). 
CONST adVarChar = 200           'Indicates a String value (Parameter object only). 
CONST adVariant = 12            'Indicates an Automation Variant (DBTYPE_VARIANT). 
CONST adVarNumeric = 139        'Indicates a numeric value (Parameter object only). 
CONST adVarWChar = 202          'Indicates a null-terminated Unicode character string (Parameter object only). 
CONST adWChar = 130             'Indicates a null-terminated Unicode character string (DBTYPE_WSTR). 
%>