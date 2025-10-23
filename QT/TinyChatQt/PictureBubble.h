#ifndef PICTUREBUBBLE_H
#define PICTUREBUBBLE_H

#include "BubbleFrame.h"
#include "global.h"

#include <QHBoxLayout>
#include <QPixmap>

class PictureBubble : public BubbleFrame
{
public:
    PictureBubble(const QPixmap &picture,
                  ChatRole role,
                  QWidget *parent = nullptr);
};

#endif // PICTUREBUBBLE_H
