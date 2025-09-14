#define CHECK(cmd) {\
  result = cmd;\
  if(result != SI_SUCCESS) {\
    sprintf(ip->result,"error during SAAPHIRE call %s",# cmd);\
    return(TCL_ERROR);\
  }\
}

#define ADDCMD(cmd) Tcl_CreateCommand(ip,#cmd,cmd ## CMD,0,0)
#define DEFCMD(cmd) int cmd ## CMD(ClientData clientData, Tcl_Interp *ip, int argc,const char *argv[])
#define ADDOBJCMD(cmd) Tcl_CreateObjCommand(ip,#cmd,cmd ## CMD,0,0)
#define DEFOBJCMD(cmd) int cmd ## CMD(ClientData clientData, Tcl_Interp *ip, int objc, Tcl_Obj *const *objv)
// We should check that str is derived from __str!
#define GETPTR(p,str,i) {\
  char __str[200];\
  sscanf(argv[i],"%x %s",&p,__str);\
}
#define GETOBJPTR(p,str,i) {\
  char __str[200];\
  sscanf(Tcl_GetString(objv[i]),"%x %s",&p,__str);\
}
#define RETURN_PTR(p,str) {char _buf[100];sprintf(_buf,"%#x %s",p,str);Tcl_AppendResult(ip,_buf,NULL);return(TCL_OK);}
