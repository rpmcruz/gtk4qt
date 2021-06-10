/* To turn the style into a plugin */

#include <qstyleplugin.h>
#include "qgtkstyle.h"

class QGtkStylePlugin : public QStylePlugin
{
public:
	QGtkStylePlugin() {}
	~QGtkStylePlugin() {}

	QStringList keys() const
	{ return QStringList() << "qgtkstyle"; }

	QStyle *create (const QString &key)
	{
		if (key == "qgtkstyle")
			return new QGtkStyle;
		return 0;
	}
};

Q_EXPORT_PLUGIN (QGtkStylePlugin)
