//
//  FLNodes.h
//

#ifndef _FLNodes_h
#define _FLNodes_h

#include <QString>
#include <QRect>

#include "smartpointer.h"

class FLComponentItem;

/****************************LAYOUT OPTIMIZATION TREE*****************/
class binaryNode : public smartable
{
    public :
        SMARTP<binaryNode> left;
        SMARTP<binaryNode> right;

        binaryNode(binaryNode* l, binaryNode* r): left(l), right(r) {}
        
        //  Representing the surface of the interface
        virtual QRect 	rectSurface() = 0;
        virtual QString renderToFaust(const QString& faustOperator, const QString& layoutIndex) = 0;
        
        int surface() {
            int surface =  rectSurface().width() * rectSurface().height();
            printf("SURFACE CALCULATED = %i\n", surface);
            return surface;
        }
};

class treeNode : public binaryNode
{
    public:
        QRect rect;
        
		treeNode(binaryNode* l, binaryNode* r): binaryNode(l,r) {}
        virtual QRect rectSurface()			{ return rect; }
};

class verticalNode : public treeNode
{
	public:
        verticalNode(binaryNode* node1, binaryNode* node2, QRect r) : treeNode(node1, node2) { rect = r; }
        
        virtual QString renderToFaust(const QString& faustOperator, const QString& layoutIndex){
            QString faustCode = "vgroup(\"["+ layoutIndex + "]\"," + left->renderToFaust(faustOperator, "1") + faustOperator + right->renderToFaust(faustOperator, "2")+")";
            
            return faustCode;
        }
};

class horizontalNode : public treeNode
{
    public:
        horizontalNode(binaryNode* node1, binaryNode* node2, QRect r) : treeNode(node1, node2) { rect = r; }
        
        virtual QString renderToFaust (const QString& faustOperator, const QString& layoutIndex) { 
            QString faustCode = "hgroup(\"["+ layoutIndex + "]\"," + left->renderToFaust(faustOperator, "1") + faustOperator + right->renderToFaust(faustOperator, "2")+")";
            return faustCode;
        } 
};

class leafNode : public binaryNode
{
    public:
        FLComponentItem* item;
        
        leafNode(FLComponentItem* i) : binaryNode(NULL, NULL) { item = i; }
        
        virtual QString renderToFaust(const QString& /*faustOperator*/, const QString& layoutIndex){
            return item->faustComponent(layoutIndex);
        }
        
        virtual QRect rectSurface() 	{ return item->rect(); }
};

binaryNode* 		createBestContainerTree(binaryNode* node1, binaryNode* node2);
QList<binaryNode*> 	createListTrees(QList<FLComponentItem*> components);
QList<binaryNode*> 	dispatchComponentOnListOfTrees(FLComponentItem* component, QList<binaryNode*> existingTrees);
binaryNode* 		calculateBestDisposition(QList<FLComponentItem*> components);


#endif
