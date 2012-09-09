#include <QMessageBox>

#include "mainwindow.h"

namespace chrbrew
{
    MainWindow::MainWindow()
    {
        resize(640, 640);

        scroll = new QScrollArea();
        scroll->setWidgetResizable(true);
        setCentralWidget(scroll);

        editor = new EditorWidget();
        scroll->setWidget(editor);

        statusBar()->showMessage(tr("img2chr - by Overkill."), 2000);
        statusBar()->setStyleSheet(
            "QStatusBar {"
            "   border-top: 1px solid #CCCCCC;"
            "   background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #DADBDE, stop: 1 #F6F7FA);"
            "   padding: 4px;"
            "   color: #777777;"
            "}"
        );

#ifdef Q_WS_WIN
        QKeySequence quitSequence(Qt::ALT + Qt::Key_F4);
#else
        QKeySequence quitSequence(QKeySequence::Quit);
#endif

        fileMenu = menuBar()->addMenu(tr("&File"));
        newAction = createAction(fileMenu, tr("&New"), tr("Create a new CHR."), QKeySequence::New);
        openAction = createAction(fileMenu, tr("&Open..."), tr("Open an existing CHR."), QKeySequence::Open);
        createSeparator(fileMenu);
        saveAction = createAction(fileMenu, tr("&Save..."), tr("Save the current CHR."), QKeySequence::Save);
        saveAsAction = createAction(fileMenu, tr("Save &As..."), tr("Save a copy of the current CHR."), QKeySequence::SaveAs);
        createSeparator(fileMenu);
        for(int i = 0; i < MaxRecentCount; ++i)
        {
            QAction* action(createAction(fileMenu, QKeySequence(Qt::ALT + Qt::Key_1 + i)));
            action->setDisabled(true);
            recentFileActions[i] = action;
            connect(action, SIGNAL(triggered()), this, SLOT(openRecentFile()));
        }
        clearRecentAction = createAction(fileMenu, tr("Clear &Recent Files"), tr("Clear all recently opened files."), QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Delete));
        updateRecentFiles();
        createSeparator(fileMenu);
        exitAction = createAction(fileMenu, tr("E&xit"), tr("Exit the program."), quitSequence);

        helpMenu = menuBar()->addMenu(tr("&Help"));
        aboutAction = createAction(helpMenu, tr("&About..."), tr("About img2chr."), QKeySequence::HelpContents);

        connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));
        connect(openAction, SIGNAL(triggered()), this, SLOT(openFile()));
        connect(saveAction, SIGNAL(triggered()), this, SLOT(saveFile()));
        connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveFileAs()));
        connect(clearRecentAction, SIGNAL(triggered()), this, SLOT(clearRecentFiles()));
        connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
        connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

        QTimer::singleShot(0, this, SLOT(newFile()));
    }

    void MainWindow::newFile()
    {
        delete editor;
        editor = new EditorWidget();
        scroll->setWidget(editor);
        setCurrentFile(QString());
    }

    void MainWindow::openFile()
    {
        QString filename(
            QFileDialog::getOpenFileName(
                this,
                tr("Open Character Set"),
                QString(),
                tr("Tilesets (*.chr);;")
            )
        );
        if(!filename.isEmpty())
        {
            readFile(filename);
        }
    }

    void MainWindow::saveFile()
    {
        if(currentFile.isEmpty())
        {
            saveFileAs();
        }
        else
        {
            writeFile(currentFile);
        }
    }

    void MainWindow::saveFileAs()
    {
        QString filename(
            QFileDialog::getSaveFileName(
                this,
                tr("Save Character Set"),
                QString(),
                tr("Character Sets (*.chr);;")
            )
        );
        if(!filename.isEmpty())
        {
            writeFile(filename);
        }
    }

    void MainWindow::openRecentFile()
    {
        QAction* action(qobject_cast<QAction*>(sender()));
        if(action)
        {
            readFile(action->data().toString());
        }
    }

    void MainWindow::clearRecentFiles()
    {
        QSettings settings;
        QStringList recentFiles(settings.value("recentFiles").toStringList());
        recentFiles.clear();
        settings.setValue("recentFiles", recentFiles);
        updateRecentFiles();
    }

    void MainWindow::about()
    {
        QMessageBox::about(this, tr("%1").arg(AppName), tr(
                "<p><strong>%1</b></strong> &ndash; <em>by Andrew G. Crowell (Overkill)</em>.</p>"
                "<p>A tool for converting images into raw tile/character sets.</p>"
                "<p><a href='https://github.com/Bananattack/img2chr/'>https://github.com/Bananattack/img2chr/</a></p>"
            ).arg(AppName)
        );
    }

    void MainWindow::readFile(const QString& filename)
    {
        delete editor;
        editor = new EditorWidget();
        scroll->setWidget(editor);
        setCurrentFile(filename);
        editor->readFile(filename);
    }

    void MainWindow::writeFile(const QString& filename)
    {
        if(editor->writeCHR(filename))
        {
            setCurrentFile(filename);
            statusBar()->showMessage(tr("File saved as %1.").arg(filename), 2000);
        }
    }

    void MainWindow::setCurrentFile(const QString& filename)
    {
        setWindowModified(false);
        if(filename.isEmpty())
        {
            setWindowFilePath("untitled.chr");
        }
        else
        {
            setWindowFilePath(filename);

            QSettings settings;
            QStringList recentFiles(settings.value("recentFiles").toStringList());
            recentFiles.removeAll(filename);
            recentFiles.prepend(filename);
            while(recentFiles.size() > MaxRecentCount)
            {
                recentFiles.removeLast();
            }

            settings.setValue("recentFiles", recentFiles);
            updateRecentFiles();
        }
        currentFile = filename;
    }

    void MainWindow::createSeparator(QMenu* menu)
    {
        menu->addSeparator();
    }

    QAction* MainWindow::createAction(QMenu* menu, const QKeySequence &shortcut)
    {
        QAction* action(new QAction(this));
        menu->addAction(action);
        action->setShortcut(shortcut);
        return action;
    }

    QAction* MainWindow::createAction(QMenu* menu, const QString& text, const QString& description, const QKeySequence &shortcut)
    {
        QAction* action(createAction(menu, shortcut));
        action->setText(text);
        action->setStatusTip(description);
        return action;
    }

    void MainWindow::updateRecentFiles()
    {
        QSettings settings;
        QStringList recentFiles(settings.value("recentFiles").toStringList());
        int recentCount = recentFiles.size() > MaxRecentCount ? MaxRecentCount : recentFiles.size();
        for(int i = 0, end = recentCount; i != end; ++i)
        {
            recentFileActions[i]->setText(tr("&%1. %2")
                .arg(i + 1)
                .arg(QFileInfo(recentFiles[i]).fileName())
            );
            recentFileActions[i]->setData(recentFiles[i]);
            recentFileActions[i]->setStatusTip(recentFiles[i]);
            recentFileActions[i]->setDisabled(false);
        }
        for(int i = recentCount, end = MaxRecentCount; i != end; ++i)
        {
            recentFileActions[i]->setText(tr("&%1.").arg(i + 1));
            recentFileActions[i]->setDisabled(true);
        }
    }
}
