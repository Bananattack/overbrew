#include <QMessageBox>

#include "mainwindow.h"

namespace img2chr
{
    MainWindow::MainWindow()
    {
        setMinimumSize(QSize(320, 240));
        setWindowTitle(tr("img2chr"));
        statusBar()->showMessage(tr("img2chr"));

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
    }

    void MainWindow::newFile()
    {
        qDebug("New file!\n");
    }

    void MainWindow::openFile()
    {
        QString filename(QFileDialog::getOpenFileName(this));
        if(!filename.isEmpty())
        {
            readFile(filename);
        }
    }

    void MainWindow::saveFile()
    {
        qDebug("Save file!\n");
    }

    void MainWindow::saveFileAs()
    {
        qDebug("Save file as!\n");
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
        QMessageBox::about(this, tr("img2chr"), tr(
                "<p><strong>img2chr</b></strong> &ndash; <em>by Andrew G. Crowell (Overkill)</em>.</p>"
                "<p>A tool for converting images into raw tile/character sets.</p>"
                "<p><a href='https://github.com/Bananattack/img2chr/'>https://github.com/Bananattack/img2chr/</a></p>"
            )
        );
    }

    void MainWindow::readFile(const QString& filename)
    {
        setCurrentFile(filename);
    }

    void MainWindow::writeFile(const QString& filename)
    {
    }

    void MainWindow::setCurrentFile(const QString& filename)
    {
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
        for(int i = 0; i < recentCount; ++i)
        {
            recentFileActions[i]->setText(tr("&%1. %2")
                .arg(i + 1)
                .arg(QFileInfo(recentFiles[i]).fileName())
            );
            recentFileActions[i]->setData(recentFiles[i]);
            recentFileActions[i]->setStatusTip(recentFiles[i]);
            recentFileActions[i]->setDisabled(false);
        }
        for(int i = recentCount; i < MaxRecentCount; ++i)
        {
            recentFileActions[i]->setText(tr("&%1.").arg(i + 1));
            recentFileActions[i]->setDisabled(true);
        }
    }
}
