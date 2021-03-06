///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_CONNPT_H
#define INCLUDED_CONNPT_H

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

#define DECLARE_CONNECTION_POINT(Interface) \
   virtual tResult Connect(Interface* pSink) = 0; \
   virtual tResult Disconnect(Interface* pSink) = 0;

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_CONNPT_H
