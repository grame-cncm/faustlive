//
//  FLComponentItem.h
//
//  Created by Sarah Denoux on 12/04/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef _FLComponentItem_h
#define _FLComponentItem_h

#include <QWidget>
#include <QVBoxLayout>

class dsp;
/****************** Different items composed in the component creator *************/
class FLComponentItem : public QWidget
{
    private:
        Q_OBJECT
    
//    Place in the Component Window for group name
        QString         fIndex;
        QString         fSource;
        dsp*            fCompiledDSP;
        QVBoxLayout*    fLayout;
        QWidget*        fCurrentWidget;
    
        QString         handleDrop(QDropEvent * event);
    
    public:
				 FLComponentItem(const QString& index, QWidget* parent = NULL);
        		 FLComponentItem(const QString& source, QRect rect, QWidget* parent = NULL);
        virtual ~FLComponentItem();
            
        QString     source();
        void        createInterfaceInRect(const QString& source);
        QString     faustComponent(const QString& layoutIndex);
    
        virtual void dropEvent ( QDropEvent * event );
        virtual void dragEnterEvent ( QDragEnterEvent * event );

	signals:

};

#endif
