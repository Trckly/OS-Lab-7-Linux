#pragma once

#include <QtCore/qglobal.h>

#if defined(DYNAMICLIB_LIBRARY)
#  define DYNAMICLIB_EXPORT Q_DECL_EXPORT
#else
#  define DYNAMICLIB_EXPORT Q_DECL_IMPORT
#endif
