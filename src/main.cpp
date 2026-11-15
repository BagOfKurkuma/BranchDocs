#include <QApplication>
#include "app/EditorController.h"
#include "ui/MainWindow.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("BranchDocs");

    branchdocs::domain::Document doc("Без назви");
    branchdocs::EditorController controller(std::move(doc));

    branchdocs::MainWindow window(&controller);
    window.show();

    return app.exec();
}
