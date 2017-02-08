#ifndef HOST_ADDRESS_H_
#define HOST_ADDRESS_H_

#include <qhostaddress.h>   // needs "QT += network!"!

/**
 * At the moment, HostAddress is an alias (typedef) for QHostAddress. If needed, this
 * can be extended in deriving HostAddress from QHostAddress.
 *
 * Note that we need to declare HostAddress as metatype, but QHostAddress is not yet declared
 * as metatype.
 */
typedef QHostAddress HostAddress;
Q_DECLARE_METATYPE(HostAddress)


// Note: QIPAddressUtils namespace declarations are located in private header qipaddress_p.h. It
// declares some very useful methods which are not accessible via QHostAddress. So, the following
// forward declarations are copied from the private header to make them public accessible.
// The declarations might have to be adapted for future Qt versions!
// AT THE MOMENT NOT USED
//namespace QIPAddressUtils
//{
//typedef quint32 IPv4Address;
//typedef quint8 IPv6Address[16];
//Q_CORE_EXPORT bool parseIp4(IPv4Address& address, const QChar* begin, const QChar* end);
//Q_CORE_EXPORT const QChar* parseIp6(IPv6Address& address, const QChar* begin, const QChar* end);
//Q_CORE_EXPORT void toString(QString& appendTo, IPv4Address address);
//Q_CORE_EXPORT void toString(QString& appendTo, IPv6Address address);
//}
//

#endif /* HOST_ADDRESS_H_ */
