#ifndef _SyntheticBuildingConstants_H_
#define _SyntheticBuildingConstants_H_

#include <QtCore/QString>

namespace SyntheticBuilding
{
  const QString SyntheticBuildingPluginFile("SyntheticBuildingPlugin");
  const QString SyntheticBuildingPluginDisplayName("Synthetic Building");
  const QString SyntheticBuildingBaseName("SyntheticBuildingPlugin");
}

/**
 * @namespace FilterParameterWidgetType
 * @brief If you create custom Filter Parameter Widgets for your classes then those need to be defined here
 */
namespace FilterParameterWidgetType
{
  const QString InitializeSyntheticVolumeWidget("InitializeSyntheticVolumeWidget");
}


#endif
