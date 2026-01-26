#pragma once
#include "includes.h"
#include "strict.h"

namespace Bocari
{
    class AutoFitPlugin : public QObject, public ccStdPluginInterface
    {
        Q_OBJECT
        Q_INTERFACES(ccStdPluginInterface)
        Q_PLUGIN_METADATA(IID "cc.plugins.stdplugin" FILE "../info.json")

    public:
        AutoFitPlugin();
        ~AutoFitPlugin() override;

        // ccPluginInterface
        void setMainAppInterface(ccMainAppInterface* app) override;
        QList<QAction*> getActions() override;

    private slots:
        void performAction();

    private:
        QAction* m_action;
        ccMainAppInterface* m_app;
    };
} // namespace Bocari
