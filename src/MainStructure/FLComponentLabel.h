//
//  FLComponentLabel.h
//

#ifndef _FLComponentLabel_h
#define _FLComponentLabel_h

#include <QLabel>

class QWidget;

/************************ Droppable Label  ***************************/ 
class MyLabel : public QLabel
{
    private:
        Q_OBJECT

    public:
        		 MyLabel(QWidget* parent = NULL) {Q_UNUSED(parent);}
        virtual ~MyLabel(){}
        
        virtual void mouseReleaseEvent( QMouseEvent * event ) {
            Q_UNUSED(event);
            emit imageClicked();
        }
        
        signals:
            void imageClicked();
};

#endif
