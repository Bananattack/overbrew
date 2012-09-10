#include "spritetablewidget.h"

namespace spritebrew
{
    SpriteTableWidget::SpriteTableWidget(int rows, int columns, QWidget* parent)
        : QTableWidget(rows, columns, parent)
    {
    }

    QStyleOptionViewItem SpriteTableWidget::viewOptions() const
    {
        QStyleOptionViewItem option = QTableWidget::viewOptions();
        option.decorationAlignment = Qt::AlignHCenter | Qt::AlignCenter;
        option.decorationPosition = QStyleOptionViewItem::Top;

        return option;
    }

    QTableWidgetItem* SpriteTableWidget::createCheckbox(int row, int column)
    {
        QTableWidgetItem* item(new QTableWidgetItem());
        item->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton));
        item->setFlags(item->flags() &~ Qt::ItemIsEditable);
        setItem(row, column, item);
        return item;
    }
}
