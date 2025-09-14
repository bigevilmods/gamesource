/************************************/
/* Browser Detect                   */
/************************************/
var NS4 = (document.layers) ? 1 : 0;
var IE4 = (document.all) ? 1 : 0;
var OP4 = (navigator.userAgent.indexOf("Opera")>=0) ? 1 : 0;
var MAC = (navigator.appVersion.indexOf("Mac") != -1);
var VER3 = (parseInt(navigator.appVersion) > 3) ? 1 : 0;

/************************************/
/* user custom settings             */
/************************************/

    MENU_OFFSET_X = 0;
    MENU_OFFSET_Y = 12;
    MENU_WIDTH  = 50;
    MENUSET_SPACING = 22;
	
	nonlinkbgcolor 	= "#C4C4C4";	// color name/code
	hoverbgcolor 	= "#C4C4C4";	// color name/code
	bgcolor      	= "#EEEEEE";	// color name/code
	background   	= "";			// picture url
	fontstyle			= "<FONT FACE=\"Verdana, Arial, Helvetica, sans-serif\" SIZE=\"2\" COLOR=\"#FFFFFF\">"
	nonlinkfontstyle	= "<FONT FACE=\"Verdana, Arial, Helvetica, sans-serif\" SIZE=\"2\" COLOR=\"#FFFFFF\">"
	
	menu_border  = 1;			// IE only
	border_color = "#000000";	// IE only
	
	arrow_pic    = "menuarrow.gif"; // url of arrow picture for submenus

/************************************/
/* globals                          */
/************************************/
layer = (IE4 && VER3) ? "document.all['L'+menu].style" : (NS4 && VER3) ? "document.layers['L'+menu]" : 0;
var timer;
var PopupLinks = new Array();

/************************************/
function SetPopupLinks() 
{
	for(i=0; i<arguments.length; i++) 
	{
        PopupLinks[i] = arguments[i];
    }
}

/************************************/
function ShowNotAvail(platformName)
{
    alert("Not available for " + platformName + ".")
}

/************************************/
function Show() 
{ 	
    if(layer) 
	{
		if(timer) clearTimeout(timer);
		for(menu=0; menu<Layer.length; menu++) 
		{ 
			if(Layer[menu])
            {
				eval(layer).visibility = "hidden";
            } 
		}

        if (arguments.length>0)
        {
            /* change links to new href */
	        for(i=0; i<PopupLinks.length; i++)
	        {
                link = document.links(i);
		        platformLinkName = link.id;
                if (PopupLinks[i]=="")
                {
                    link.href = "Javascript:ShowNotAvail('"+platformLinkName+"')";
                    link.style.color = "#999999";
                    link.style.textDecorationNone=true;
                }
                else
                {
                    re = new RegExp("(.+)(.htm.?)(#?.*)","ig");
                    re.exec(PopupLinks[i]);
                    fileNameBase = RegExp.$1
                    fileNameExt = RegExp.$2
                    anchor = RegExp.$3
                    link.href = fileNameBase + "_[" + platformLinkName + "]" + fileNameExt + anchor;
                    link.style.color = "#003399";
                    link.style.textDecorationNone=true;
                }
            }

		    for(i=0; i<arguments.length; i++) 
		    { 
			    menu=arguments[i];
			    /* make position of popup menu relative to mouse position: */
                if (OP4)
                {
			        eval(layer).left = event.clientX - event.offsetX + MENU_OFFSET_X;
			        eval(layer).top  = event.clientY - event.offsetY + MENU_OFFSET_Y;
                }
                else
                {
			        eval(layer).left = event.clientX + document.body.scrollLeft;
			        eval(layer).top  = event.clientY + document.body.scrollTop;
                }
                // alert("event.offsetX="+event.offsetX+"\nevent.offsetY="+event.offsetY)
			    eval(layer).visibility = "visible"; 
		    }
        }
	}
}

/************************************/
function Hide() { timer = setTimeout("Show()", 500); }

/************************************/
if(IE4) { if(!background) { background=bgcolor; } else { background = "url("+background+")"; } }

/************************************/
function LayerSpecs(MenuItemNum) 
{ 
	if(VER3)
	{
		Width=MENU_WIDTH;
		this.menuItemNum  = MenuItemNum;
		this.left  = 0;
		this.top   = 0;
		this.width   = Width;
		this.info  = "";
		T=0;
		for(i=1; i<arguments.length; i++)
		{
			if(IE4) 
				{ this.info += "<TR><TD WIDTH="+Width+" onMouseOver='this.bgColor=\""+hoverbgcolor+"\"' onMouseOut='this.bgColor=\"\"'>"+arguments[i]+"</TD></TR>"; }
			else 
			{
				if(arguments[i].charAt(0)=='<' && arguments[i].charAt(1).toUpperCase()=='A')	// a link
				    this.info += "<LAYER onMouseOver='this.bgColor=\""+hoverbgcolor+"\"' onMouseOut='this.bgColor=\""+bgcolor+"\"' WIDTH="+Width+" POSITION=RELATIVE TOP="+T+">&nbsp;"+fontstyle+arguments[i]+"</FONT></LAYER>";
//                    this.info += "<LAYER onMouseOver='this.bgColor=\""+hoverbgcolor+"\"' onMouseOut='this.bgColor=\""+bgcolor+"\"' WIDTH="+Width+" POSITION=RELATIVE TOP="+T+">&nbsp;"+arguments[i]+"</LAYER>";
				else		// not a link		
					this.info += "<LAYER bgColor=\""+nonlinkbgcolor+"\" WIDTH="+Width+" POSITION=RELATIVE TOP="+T+">&nbsp;"+nonlinkfontstyle+arguments[i]+"</FONT></LAYER>";
//                    this.info += "<LAYER onMouseOver='this.bgColor=\""+hoverbgcolor+"\"' onMouseOut='this.bgColor=\""+bgcolor+"\"' WIDTH="+Width+" POSITION=RELATIVE TOP="+T+">&nbsp;"+arguments[i]+"</LAYER>";
//                    this.info += "<LAYER WIDTH="+Width+" POSITION=RELATIVE TOP="+T+">&nbsp;"+arguments[i]+"</LAYER>";
            }
			T+=(MENU_OFFSET_Y*2);
		}
	} 
}

Layer = new Array();
arrow = "<IMG SRC='"+arrow_pic+"' WIDTH=6 HEIGHT=10 BORDER=0 ALT=''>";





