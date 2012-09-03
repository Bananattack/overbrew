#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>

namespace img2chr
{
    class MainWindow : public QMainWindow
    {
        Q_OBJECT
        private:
            static const int MaxRecentCount = 5;

        public:
            MainWindow();

        private slots:
            void newFile();
            void openFile();
            void saveFile();
            void saveFileAs();
            void openRecentFile();
            void clearRecentFiles();
            void about();

        private:
            void readFile(const QString& filename);
            void writeFile(const QString& filename);
            void setCurrentFile(const QString& filename);
            void createSeparator(QMenu* menu);
            QAction* createAction(QMenu* menu, const QKeySequence &shortcut);
            QAction* createAction(QMenu* menu, const QString& text, const QString& description, const QKeySequence &shortcut);
            void updateRecentFiles();

            QMenu* fileMenu;
            QMenu* helpMenu;
            QAction* newAction;
            QAction* openAction;
            QAction* saveAsAction;
            QAction* saveAction;
            QAction* recentFileActions[MaxRecentCount];
            QAction* clearRecentAction;
            QAction* exitAction;
            QAction* aboutAction;
            QString currentFile;
    };
}

#endif
