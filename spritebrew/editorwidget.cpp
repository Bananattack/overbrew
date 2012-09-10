#include <QMessageBox>

#include "editorwidget.h"

namespace spritebrew
{
    EditorWidget::EditorWidget()
    {
        QVBoxLayout* mainLayout(new QVBoxLayout());
        mainLayout->setAlignment(Qt::AlignTop);
        setLayout(mainLayout);

        if(QGroupBox* group = new QGroupBox(tr("Character Set")))
        {
            mainLayout->addWidget(group);

            QVBoxLayout* groupLayout(new QVBoxLayout());
            groupLayout->setAlignment(Qt::AlignCenter | Qt::AlignTop);
            group->setLayout(groupLayout);

            QHBoxLayout* browseLayout(new QHBoxLayout());
            browseLayout->setAlignment(Qt::AlignLeft);
            groupLayout->addLayout(browseLayout);

            imageFilenameLabel = new QLabel(tr("This sprite has no character set yet."));
            browseLayout->addWidget(imageFilenameLabel);

            imageBrowseButton = new QPushButton(tr("&Import..."));
            imageBrowseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            imageBrowseButton->setAutoDefault(true);
            browseLayout->addWidget(imageBrowseButton);

            imageLabel = new QLabel();
            groupLayout->addWidget(imageLabel);
        }
        if(QHBoxLayout* rowLayout = new QHBoxLayout())
        {
            mainLayout->addLayout(rowLayout);
            if(QVBoxLayout* columnLayout = new QVBoxLayout())
            {
                rowLayout->addLayout(columnLayout);
                if(QGroupBox* group = new QGroupBox(tr("Palettes")))
                {
                    columnLayout->addWidget(group);

                    QGridLayout* groupLayout(new QGridLayout());
                    groupLayout->setAlignment(Qt::AlignTop);
                    group->setLayout(groupLayout);

                    for(int i = 0; i != 8; ++i)
                    {
                        QLabel* label = new QLabel(tr("Palette %1 is unused.").arg(i));
                        groupLayout->addWidget(label, i, 0);

                        QPushButton* button = new QPushButton(tr("&Import..."));
                        button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                        button->setAutoDefault(true);
                        groupLayout->addWidget(button, i, 1);
                    }
                }
            }
            if(QVBoxLayout* columnLayout = new QVBoxLayout())
            {
                rowLayout->addLayout(columnLayout);
                if(QGroupBox* group = new QGroupBox(tr("Parts")))
                {
                    columnLayout->addWidget(group);

                    QGridLayout* groupLayout(new QGridLayout());
                    group->setLayout(groupLayout);

                    groupLayout->addWidget(new QLabel(tr("Total Used")), 0, 0);
                    groupLayout->addWidget(new QLabel("1"), 0, 1, 1, 2);

                    groupLayout->addWidget(new QLabel(tr("Current Part")), 1, 0);
                    groupLayout->addWidget(new QSpinBox(), 1, 1, 1, 2);

                    groupLayout->addWidget(new QPushButton(tr("Add")), 2, 0);
                    groupLayout->addWidget(new QPushButton(tr("Remove")), 2, 1);
                    groupLayout->addWidget(new QPushButton(tr("Move...")), 2, 2);
                }
                if(QGroupBox* group = new QGroupBox(tr("Part Properties")))
                {
                    columnLayout->addWidget(group);

                    QGridLayout* groupLayout(new QGridLayout());
                    group->setLayout(groupLayout);

                    groupLayout->addWidget(new QLabel(tr("X")), 0, 0);
                    groupLayout->addWidget(new QSpinBox(), 0, 1);
                    groupLayout->addWidget(new QLabel(tr("Tile")), 0, 2);
                    groupLayout->addWidget(new QSpinBox(), 0, 3);
                    groupLayout->addWidget(new QLabel(tr("Y")), 1, 0);
                    groupLayout->addWidget(new QSpinBox(), 1, 1);
                    groupLayout->addWidget(new QLabel(tr("Palette")), 1, 2);
                    groupLayout->addWidget(new QSpinBox(), 1, 3);
                }
                if(QGroupBox* group = new QGroupBox(tr("Part Attributes")))
                {
                    columnLayout->addWidget(group);

                    QVBoxLayout* groupLayout(new QVBoxLayout());
                    group->setLayout(groupLayout);

                    groupLayout->addWidget(new QCheckBox(tr("Horizontal Flip")));
                    groupLayout->addWidget(new QCheckBox(tr("Vertical Flip")));
                    groupLayout->addWidget(new QCheckBox(tr("Behind Background")));
                }
            }
        }
        if(QGroupBox* group = new QGroupBox(tr("Preview")))
        {
            mainLayout->addWidget(group);

            QVBoxLayout* groupLayout(new QVBoxLayout());
            groupLayout->setAlignment(Qt::AlignCenter | Qt::AlignTop);
            group->setLayout(groupLayout);
        }
        connect(imageBrowseButton, SIGNAL(clicked()), this, SLOT(browse()));
    }

    void EditorWidget::browse()
    {
        QString filename(
            QFileDialog::getOpenFileName(
                this,
                tr("Import Character Set"),
                QString(),
                tr(
                    ";;Character Sets (*.chr)"
                    ";;All Files (*.*)"
                )
            )
        );
        if(!filename.isEmpty())
        {
            QString suffix(QFileInfo(filename).suffix());
            if(suffix == "chr")
            {
                if(readCHR(filename))
                {
                    setupImage(filename);
                }
            }
            else
            {
                QMessageBox::critical(this->parentWidget(), tr("Import Failed"), tr("'%1' has unrecognized file extension.").arg(filename));
            }
        }
    }

    bool EditorWidget::readCHR(const QString &filename)
    {
        QFile file(filename);
        if(!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this->parentWidget(), tr("Import Failed"), tr("'%1' could not be imported as a CHR.").arg(filename));
            return false;
        }

        int tiles = file.size() / 16;
        if(tiles == 0)
        {
            QMessageBox::critical(this->parentWidget(), tr("Import Failed"), tr("'%1' has no tiles! (File size is %2 byte(s))").arg(filename).arg(file.size()));
            return false;
        }

        // Try to fit this in a nice rectangular region, so it's easier to view.
        int columns = 0;
        int rows = 0;
        for(int i = 16; i != 1; i /= 2)
        {
            if(tiles % i == 0)
            {
                if(i >= 8)
                {
                    columns = i;
                    rows = tiles / i;
                }
                else
                {
                    rows = i;
                    columns = tiles / i;
                }
                break;
            }
        }
        if(columns == 0)
        {
            columns = tiles;
            rows = 1;
        }

        image = QImage(columns * 8, rows * 8, QImage::Format_Indexed8);
        image.setColorCount(4);
        for(int i = 0; i != 4; ++i)
        {
            image.setColor(i, getPaletteColor(i));
        }
        image.fill(0);

        QByteArray bytes(file.read(file.size()));
        for(int r = 0; r != rows; ++r)
        {
            for(int c = 0; c != columns; ++c)
            {
                for(int j = 0; j != 8; ++j)
                {
                    int index = ((r * columns + c) * 8 + j) * 2;
                    unsigned char low = bytes[index];
                    unsigned char high = bytes[index + 1];
                    for(int i = 0; i != 8; ++i)
                    {
                        image.setPixel(
                            c * 8 + i,
                            r * 8 + j,
                            ((high & (1 << (7 - i))) ? 2 : 0) | ((low & (1 << (7 - i))) ? 1 : 0)
                        );
                    }
                }
            }
        }

        return true;
    }

    void EditorWidget::setupImage(const QString& filename)
    {
        imageFilenameLabel->setText(tr("<b>%1</b>").arg(QFileInfo(filename).fileName()));
        imageLabel->setPixmap(QPixmap::fromImage(image));
    }

    QRgb EditorWidget::getPaletteColor(int i)
    {
        switch(i)
        {
            case 0: return qRgb(0x50, 0x50, 0x50); break;
            case 1: return qRgb(0xA0, 0xA0, 0xA0); break;
            case 2: return qRgb(0xD0, 0xD0, 0xD0); break;
            case 3: return qRgb(0xFF, 0xFF, 0xFF); break;
            default: return qRgb(0xFF, 0x00, 0xFF); break;
        }
    }
}
