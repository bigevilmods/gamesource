<!--#include file="bar.inc" -->

<html>
<head>
<link rel="stylesheet" href="navstyle.css">
</head>

<body topmargin="0" leftmargin="0" onload="SetToolBar(3);">
<form name="QForm" action="http://tools.eac.ea.com/search/default.asp?Action=Search" method=POST target="right">
  <!--     -->
  <table border="0" cellpadding="1" cellspacing="5">
    <tr bgcolor="#E0E0E0">
      <td colspan="2"><font size="1" face="Arial" color="#000000"><strong>Type
        in the keyword(s) to find:</strong></font></td>
    </tr>
    <tr>
      <td colspan="2"><font size="1" face="Arial" color="#000000">
        <input type="TEXT" name="SearchString" size="25" maxlength="100" value="">
        </font> </td>
    </tr>
    <tr align="right">
      <td align="left"><font color="#000000" face="Arial"><strong><font size="1"><b><font color="#FFFFFF"><a href="http://tools.eac.ea.com/search/ixtiphlp.htm" target="_blank">Tips
        for Searching</a></font></b><font color="#FFFFFF"></font></font></strong></font>
      </td>
      <td>
        <input type="HIDDEN" name="DocAuthorRestriction" value="">
        <input type="HIDDEN" name="FSRest"       value=" &lt; ">
        <input type="HIDDEN" name="FSRestVal"    value="any">
        <input type="HIDDEN" name="FSRestOther"  value="">
        <input type="HIDDEN" name="sc"        value="EACore/$projectnumber">
        <input type="HIDDEN" name="FMModDate"    value="">
        <input type="HIDDEN" name="SortBy"       value="rank[d]">
        <input type="HIDDEN" name="ColChoice"    value="1">
      </td>
    </tr>
    <tr> </tr>
  </table>
</form>
</body>
</html>

