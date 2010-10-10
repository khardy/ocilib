/*
   +----------------------------------------------------------------------+   
   |                                                                      |
   |                     OCILIB - C Driver for Oracle                     |
   |                                                                      |
   |                      (C Wrapper for Oracle OCI)                      |
   |                                                                      |
   +----------------------------------------------------------------------+
   |                      Website : http://www.ocilib.net                 |
   +----------------------------------------------------------------------+
   |               Copyright (c) 2007-2010 Vincent ROGIER                 |
   +----------------------------------------------------------------------+
   | This library is free software; you can redistribute it and/or        |
   | modify it under the terms of the GNU Lesser General Public           |
   | License as published by the Free Software Foundation; either         |
   | version 2 of the License, or (at your option) any later version.     |
   |                                                                      |
   | This library is distributed in the hope that it will be useful,      |
   | but WITHOUT ANY WARRANTY; without even the implied warranty of       |
   | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    |
   | Lesser General Public License for more details.                      |
   |                                                                      |
   | You should have received a copy of the GNU Lesser General Public     |
   | License along with this library; if not, write to the Free           |
   | Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.   |
   +----------------------------------------------------------------------+
   |          Author: Vincent ROGIER <vince.rogier@ocilib.net>            |
   +----------------------------------------------------------------------+ 
*/

/* ------------------------------------------------------------------------ *
 * $Id: event.c, v 3.8.0 2010-10-09 19:30 Vincent Rogier $
 * ------------------------------------------------------------------------ */

#include "ocilib_internal.h"

/* ************************************************************************ *
 *                             PRIVATE FUNCTIONS
 * ************************************************************************ */

/* ************************************************************************ *
 *                            PUBLIC FUNCTIONS
 * ************************************************************************ */

/* ------------------------------------------------------------------------ *
 * OCI_EnqueueCreate
 * ------------------------------------------------------------------------ */

OCI_Enqueue * OCI_API OCI_EnqueueCreate(OCI_TypeInfo *typinf, const mtext *name)
{
    OCI_Enqueue *enqueue = NULL;
    boolean res  = TRUE;

    OCI_CHECK_INITIALIZED(NULL);

    OCI_CHECK_PTR(OCI_IPC_TYPE_INFO, typinf, NULL);
    OCI_CHECK_PTR(OCI_IPC_STRING, name, NULL);

    /* allocate message structure */

    enqueue = (OCI_Enqueue *) OCI_MemAlloc(OCI_IPC_ENQUEUE, sizeof(*enqueue), 
                                       (size_t) 1, TRUE);

    if (enqueue != NULL)
    {
        enqueue->typinf  = typinf;
        enqueue->name    = mtsdup(name);

        /* allocate enqueue options handle */

        res = (OCI_SUCCESS == OCI_DescriptorAlloc((dvoid * ) OCILib.env, 
                                                  (dvoid **) &enqueue->opth,
                                                  OCI_DTYPE_AQENQ_OPTIONS, 
                                                  (size_t) 0, (dvoid **) NULL));
    }
    else
        res = FALSE;

    if (res == FALSE)
    {
        OCI_EnqueueFree(enqueue);
        enqueue = NULL;
    }

    return enqueue;
}

/* ------------------------------------------------------------------------ *
 * OCI_EnqueueFree
 * ------------------------------------------------------------------------ */

boolean OCI_API OCI_EnqueueFree(OCI_Enqueue *enqueue)
{
    OCI_CHECK_PTR(OCI_IPC_ENQUEUE, enqueue, FALSE);

    OCI_DescriptorFree((dvoid *) enqueue->opth, OCI_DTYPE_AQENQ_OPTIONS);

    OCI_FREE(enqueue->name);
    OCI_FREE(enqueue);

    return TRUE;
}

/* ------------------------------------------------------------------------ *
 * OCI_EnqueuePut
 * ------------------------------------------------------------------------ */

boolean OCI_API OCI_EnqueuePut(OCI_Enqueue *enqueue, OCI_Msg *msg)
{
    boolean res  = TRUE;
    void  *ostr  = NULL;
    int    osize = -1;

    void *payload = NULL;
    void *ind     = NULL;

    OCI_CHECK_PTR(OCI_IPC_ENQUEUE, enqueue, FALSE);
    OCI_CHECK_PTR(OCI_IPC_MSG    , msg    , FALSE);

    OCI_CHECK_COMPAT(enqueue->typinf->con, enqueue->typinf == msg->typinf, FALSE);

    ostr = OCI_GetInputMetaString(enqueue->name, &osize);

    if (mtscmp(enqueue->typinf->name, OCI_RAW_OBJECT_TYPE) != 0)
    {
        payload = &msg->obj->handle;
        ind     = &msg->obj->tab_ind;
    }
    else
    {
        payload = &msg->raw;
        ind     = NULL;
    }


    OCI_CALL2
    (
        res, enqueue->typinf->con,

        OCIAQEnq(enqueue->typinf->con->cxt, enqueue->typinf->con->err,
                 ostr, enqueue->opth, msg->proph, enqueue->typinf->tdo,
                 payload, ind, &msg->id, OCI_DEFAULT);
    )

    OCI_RESULT(res);

    return res;
}

/* ------------------------------------------------------------------------ *
 * OCI_EnqueueSetVisibility
 * ------------------------------------------------------------------------ */

boolean OCI_API OCI_EnqueueSetVisibility(OCI_Enqueue *enqueue, 
                                         unsigned int visibility)
{
    boolean res = TRUE;
    ub4 value   = (ub4) visibility;

    OCI_CHECK_PTR(OCI_IPC_ENQUEUE, enqueue, FALSE);

    OCI_CALL2
    (
        res, enqueue->typinf->con,

        OCIAttrSet((dvoid *) enqueue->opth, 
                   (ub4    ) OCI_DTYPE_AQENQ_OPTIONS,
                   (dvoid *) &value, 
                   (ub4    ) 0,
                   (ub4    ) OCI_ATTR_VISIBILITY, 
                   enqueue->typinf->con->err)
    )

    OCI_RESULT(res);

    return res;
}

/* ------------------------------------------------------------------------ *
 * OCI_EnqueueGetVisibility
 * ------------------------------------------------------------------------ */

unsigned int OCI_API OCI_EnqueueGetVisibility(OCI_Enqueue *enqueue)
{
    boolean res = TRUE;
    ub4 ret = 0;

    OCI_CHECK_PTR(OCI_IPC_ENQUEUE, enqueue, FALSE);

    OCI_CALL2
    (
        res, enqueue->typinf->con,

        OCIAttrGet((dvoid *) enqueue->opth, 
                   (ub4    ) OCI_DTYPE_AQENQ_OPTIONS,
                   (dvoid *) &ret, 
                   (ub4   *) NULL,
                   (ub4    ) OCI_ATTR_VISIBILITY, 
                   enqueue->typinf->con->err)
    )


    OCI_RESULT(res);

    return (int) ret;
}

/* ------------------------------------------------------------------------ *
 * OCI_EnqueueSetDeviation
 * ------------------------------------------------------------------------ */

boolean OCI_API OCI_EnqueueSetSequenceDeviation(OCI_Enqueue *enqueue, 
                                                unsigned int sequence)
{
    boolean res = TRUE;
    ub4 value   = (ub4) sequence;

    OCI_CHECK_PTR(OCI_IPC_ENQUEUE, enqueue, FALSE);

    OCI_CALL2
    (
        res, enqueue->typinf->con,

        OCIAttrSet((dvoid *) enqueue->opth, 
                   (ub4    ) OCI_DTYPE_AQENQ_OPTIONS,
                   (dvoid *) &value, 
                   (ub4    ) 0,
                   (ub4    ) OCI_ATTR_SEQUENCE_DEVIATION, 
                   enqueue->typinf->con->err)
    )

    OCI_RESULT(res);

    return res;
}

/* ------------------------------------------------------------------------ *
 * OCI_EnqueueGetSequenceDeviation
 * ------------------------------------------------------------------------ */

unsigned int OCI_API OCI_EnqueueGetSequenceDeviation(OCI_Enqueue *enqueue)
{
    boolean res = TRUE;
    ub4 ret = 0;

    OCI_CHECK_PTR(OCI_IPC_ENQUEUE, enqueue, FALSE);

    OCI_CALL2
    (
        res, enqueue->typinf->con,

        OCIAttrGet((dvoid *) enqueue->opth, 
                   (ub4    ) OCI_DTYPE_AQENQ_OPTIONS,
                   (dvoid *) &ret, 
                   (ub4   *) NULL,
                   (ub4    ) OCI_ATTR_SEQUENCE_DEVIATION, 
                   enqueue->typinf->con->err)
    )


    OCI_RESULT(res);

    return (int) ret;
}


/* ------------------------------------------------------------------------ *
 * OCI_EnqueueSetRelativeMsgID
 * ------------------------------------------------------------------------ */

boolean OCI_API OCI_EnqueueSetRelativeMsgID(OCI_Enqueue *enqueue, 
                                            const void *msg_id, 
                                            unsigned int len)
{
    boolean res    = TRUE;
    OCIRaw  *value = NULL;

    OCI_CHECK_PTR(OCI_IPC_ENQUEUE, enqueue, FALSE);

    OCI_CALL2
    (
        res, enqueue->typinf->con,
        
        OCIRawAssignBytes(OCILib.env, enqueue->typinf->con->err, 
                          (ub1*) msg_id, (ub4) len, (OCIRaw **) &value)
    )

    OCI_CALL2
    (
        res, enqueue->typinf->con,

        OCIAttrSet((dvoid *) enqueue->opth, 
                   (ub4    ) OCI_DTYPE_AQENQ_OPTIONS,
                   (dvoid *) &value, 
                   (ub4    ) 0,
                   (ub4    ) OCI_ATTR_RELATIVE_MSGID, 
                   enqueue->typinf->con->err)
    )

    OCI_RESULT(res);

    return res;
}

/* ------------------------------------------------------------------------ *
 * OCI_EnqueueSetRelativeMsgID
 * ------------------------------------------------------------------------ */

boolean OCI_API OCI_EnqueueGetRelativeMsgID(OCI_Enqueue *enqueue, 
                                            void *msg_id, 
                                            unsigned int len)
{
    boolean res    = TRUE;
    OCIRaw  *value = NULL;

    OCI_CHECK_PTR(OCI_IPC_ENQUEUE, enqueue, FALSE);

    OCI_CALL2
    (
        res, enqueue->typinf->con,

        OCIAttrGet((dvoid *) enqueue->opth, 
                   (ub4    ) OCI_DTYPE_AQENQ_OPTIONS,
                   (dvoid *) &value, 
                   (ub4   *) NULL,
                   (ub4    ) OCI_ATTR_RELATIVE_MSGID, 
                   enqueue->typinf->con->err)
    )

    if (value != NULL)
    {
        ub4 raw_len = 0;
    
        raw_len = OCIRawSize(OCILib.env, value);

        if (len > raw_len)
            len = raw_len;

        memcpy(msg_id, OCIRawPtr(OCILib.env, value), (size_t) len);
    }

    OCI_RESULT(res);

    return res;
}
