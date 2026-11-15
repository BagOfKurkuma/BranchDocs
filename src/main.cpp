#include <QApplication>
#include <QStandardPaths>
#include <filesystem>
#include <memory>

#include "app/EditorController.h"
#include "app/RepositoryFactory.h"
#include "ui/MainWindow.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("BranchDocs");

    const QString docsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/BranchDocs";
    const std::filesystem::path dataDir(docsPath.toStdString());
    std::filesystem::create_directories(dataDir);

    auto factory = std::make_shared<branchdocs::RepositoryFactory>(dataDir);
    const auto initialType = factory->readConfig();

    branchdocs::domain::Document doc("Без назви");
    branchdocs::EditorController controller(factory, initialType, std::move(doc));
    controller.loadHistoryFromRepository();

    branchdocs::MainWindow window(&controller);
    window.show();
    return app.exec();
}