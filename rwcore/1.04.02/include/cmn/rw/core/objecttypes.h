// (c) Electronic Arts. All Rights Reserved.

/// \file
/// RenderWare object type identifiers.

#ifndef RW_OBJECTTYPES_H
#define RW_OBJECTTYPES_H

/*************************************************************************************************************
 Includes
 */

/*************************************************************************************************************
 Defines
 */

/*************************************************************************************************************
 Enums
 */

enum RwObjectType
{
    RWOBJECTTYPE_NULL                   = RWMAKEOBJECTTYPE(rwCOMPONENTID_NA,   0x00),
    RWOBJECTTYPE_NA                     = RWMAKEOBJECTTYPE(rwCOMPONENTID_CORE, 0x00),
    RWOBJECTTYPE_ARENA                  = RWMAKEOBJECTTYPE(rwCOMPONENTID_CORE, 0x01),

    // Binary data in target specific form
    RWOBJECTTYPE_RAW                    = RWMAKEOBJECTTYPE(rwCOMPONENTID_CORE, 0x02),

    // Support for arena subreferences
    RWOBJECTTYPE_SUBREFERENCE           = RWMAKEOBJECTTYPE(rwCOMPONENTID_CORE, 0x03),

    // Built-in section types
    RWOBJECTTYPE_SECTIONMANIFEST          = RWMAKEOBJECTTYPE(rwCOMPONENTID_CORE, 0x04),
    RWOBJECTTYPE_SECTIONTYPES             = RWMAKEOBJECTTYPE(rwCOMPONENTID_CORE, 0x05),
    RWOBJECTTYPE_SECTIONEXTERNALARENAS    = RWMAKEOBJECTTYPE(rwCOMPONENTID_CORE, 0x06),
    RWOBJECTTYPE_SECTIONSUBREFERENCES     = RWMAKEOBJECTTYPE(rwCOMPONENTID_CORE, 0x07),
    RWOBJECTTYPE_SECTIONATOMS             = RWMAKEOBJECTTYPE(rwCOMPONENTID_CORE, 0x08),
    RWOBJECTTYPE_DEFARENAIMPORTS          = RWMAKEOBJECTTYPE(rwCOMPONENTID_CORE, 0x09),
    RWOBJECTTYPE_DEFARENAEXPORTS          = RWMAKEOBJECTTYPE(rwCOMPONENTID_CORE, 0x0a),
    RWOBJECTTYPE_DEFARENATYPES            = RWMAKEOBJECTTYPE(rwCOMPONENTID_CORE, 0x0b),
    RWOBJECTTYPE_DEFARENADEFINEDARENAID   = RWMAKEOBJECTTYPE(rwCOMPONENTID_CORE, 0x0c),
    RWOBJECTTYPE_ATTRIBUTEPACKET          = RWMAKEOBJECTTYPE(rwCOMPONENTID_CORE, 0x0d),
    RWOBJECTTYPE_ATTRIBUTEPACKET_DELEGATE = RWMAKEOBJECTTYPE(rwCOMPONENTID_CORE, 0x0e),

    // rw::BitTable object
    RWOBJECTTYPE_BITTABLE                 = RWMAKEOBJECTTYPE(rwCOMPONENTID_CORE, 0x0f),

    // rw:core::atom::StaticAtomTable
    RWOBJECTTYPE_ARENALOCALATOMTABLE      = RWMAKEOBJECTTYPE(rwCOMPONENTID_CORE, 0x10),

    // Base resource types for all platforms
    RWOBJECTTYPE_BASERESOURCE_START       = RWMAKEOBJECTTYPE(rwCOMPONENTID_CORE, 0x30),
    RWOBJECTTYPE_BASERESOURCE_RESERVEDTO  = RWMAKEOBJECTTYPE(rwCOMPONENTID_CORE, 0x3f),

    // note: When adding a new type here, remember to update rw::conditioning::core::GetCoreTypeName
    //       in rwxgraphcoreobject.hpp

    RWOBJECTTYPE_FORCEENUMSIZEINT   = RWFORCEENUMSIZEINT
};

/*************************************************************************************************************
 Typedef Enums
 */

typedef enum RwObjectType   RwObjectType;

/*************************************************************************************************************
 Typedef Structs + Unions
 */

/*************************************************************************************************************
 Typedef Function Pointers
 */

/*************************************************************************************************************
 Structs + Unions
 */

/*************************************************************************************************************
 Global Variables
 */

/*************************************************************************************************************
 Internal Functions Prototypes
 */

/*************************************************************************************************************
 External Functions Prototypes
 */

#endif /* RW_OBJECTTYPES_H */
