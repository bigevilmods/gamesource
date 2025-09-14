<SCRIPT Language="JavaScript">
function SetToolBar(itemNum)
{
    dirPos = document.location.href.lastIndexOf("/");
    dirPath = document.location.href.substring(0,dirPos+1)

    switch (itemNum)
    {
        case 1:
            parent.frames[0].location = dirPath+"helptoc2bar1.htm";
            break;
        case 2:
            parent.frames[0].location = dirPath+"helptoc2bar2.htm";
            break;
        case 3:
            parent.frames[0].location = dirPath+"helptoc2bar3.htm";
            break;
        default:
            break;
    }
}
</SCRIPT>
