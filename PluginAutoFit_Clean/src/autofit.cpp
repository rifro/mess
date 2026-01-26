#include "autofit.h"
#include "autofit_impl.h"
#include "includes.h"
#include "strict.h"

namespace Bocari
{
    AutoFitPlugin::AutoFitPlugin() : m_action(nullptr), m_app(nullptr) {}
    AutoFitPlugin::~AutoFitPlugin() = default;

    void AutoFitPlugin::setMainAppInterface(ccMainAppInterface* app)
    {
        m_app = app;
    }

    QList<QAction*> AutoFitPlugin::getActions()
    {
        if (!m_action)
        {
            m_action = new QAction(QString("AutoFit"), this);
            m_action->setToolTip(QString("Automatically fit geometric primitives"));
            connect(m_action, &QAction::triggered, this, &AutoFitPlugin::performAction);
        }
        return {m_action};
    }

    void AutoFitPlugin::performAction()
    {
        if (!m_app) return;

        // Get the selected point cloud
        const ccHObject::Container& selectedEntities = m_app->getSelectedEntities();
        ccPointCloud* selectedCloud = selectedEntities.empty() ? nullptr : qobject_cast<ccPointCloud*>(selectedEntities[0]);

        if (!selectedCloud)
        {
            m_app->dispToConsole("AutoFit: Please select a point cloud.", ccMainAppInterface::ERR_CONSOLE_MESSAGE);
            return;
        }

        try
        {
            m_app->dispToConsole("AutoFit: Starting analysis...", ccMainAppInterface::STD_CONSOLE_MESSAGE);

            // The PIMPL idiom in action: the plugin delegates all work to the implementation class.
            auto impl = std::make_unique<AutoFitImpl>();
            impl->performFit(m_app, selectedCloud);

            m_app->dispToConsole("AutoFit: Analysis complete.", ccMainAppInterface::STD_CONSOLE_MESSAGE);
            // In a real implementation, you would now add the fitted objects to the DB tree.
        }
        catch (const std::exception& e)
        {
            m_app->dispToConsole(QString("AutoFit Error: %1").arg(e.what()), ccMainAppInterface::ERR_CONSOLE_MESSAGE);
        }
    }

} // namespace Bocari

// The macro that makes the plugin loadable by CloudCompare
Q_EXPORT_PLUGIN2(AutoFit, Bocari::AutoFitPlugin)
