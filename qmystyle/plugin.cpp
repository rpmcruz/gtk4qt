/* To turn the style into a plugin */

#include <qstyleplugin.h>
#include "qmystyle.h"

class QMyStylePlugin : public QStylePlugin
{
public:
	QMyStylePlugin() {}
	~QMyStylePlugin() {}

	QStringList keys() const
	{ return QStringList() << "qmystyle"; }

	QStyle *create (const QString &key)
	{
		if (key == "qmystyle")
			return new QMyStyle;
		return 0;
	}
};

Q_EXPORT_PLUGIN (QMyStylePlugin)
