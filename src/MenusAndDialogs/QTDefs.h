
#pragma once

#include <QtGlobal>

#if QT_VERSION >= 0x050600
	#define QTNEWPRIMARYSCREEN
#endif

#if QT_VERSION >= 0x060000
	#define QT6
#endif

#if QT_VERSION >= 0x040500
	#define QTNEWCONFIRMOVERWRITE
#endif

// QNetworkReply::error(NetworkError) was renamed errorOccurred in Qt 5.15 and removed in Qt 6
#if QT_VERSION >= 0x050F00
	#define QNETWORKREPLY_ERROR_SIGNAL SIGNAL(errorOccurred(QNetworkReply::NetworkError))
#else
	#define QNETWORKREPLY_ERROR_SIGNAL SIGNAL(error(QNetworkReply::NetworkError))
#endif

