#include "editor.h"

Editor::Editor(QWidget *parent)
	: QTextEdit(parent)
{
	action = new QAction(this);
	action->setCheckable(true);
	connect(action,SIGNAL(triggered()),this,SLOT(show()));
	connect(action,SIGNAL(triggered()),this,SLOT(setFocus()));

	isUntitled = true;//记录文件是否已经存在

	connect(document(),SIGNAL(contentsChanged()),this,SLOT(documentWasModified()));//文件内容发生改变时，触发槽

	setWindowIcon(QPixmap("C:/CppWorkspace/editor/images/document.png"));//设置子窗口图标
	setWindowTitle("[*]");
	setAttribute(Qt::WA_DeleteOnClose);//窗口关闭时删除
}
void Editor::newFile()
{
	static int documentNumber = 1;//记录打开子窗口的个数

	curFile = tr("document%1.txt").arg(documentNumber);
	setWindowTitle(curFile + "[*]");
	action->setText(curFile);
	isUntitled = true;
	++documentNumber;
}

bool Editor::save()
{
	if (isUntitled){
		return saveAs();
	} else {
		return saveFile(curFile);
	}
}

bool Editor::saveAs()
{
	QString fileName = QFileDialog::getSaveFileName(this,tr("Save As"),curFile);
	if(fileName.isEmpty())
		return false;

	return saveFile(fileName);
}

QSize Editor::sizeHint() const
{
	return QSize(72 * fontMetrics().width('x'),25 * fontMetrics().lineSpacing());
}

Editor *Editor::open(QWidget *parent)//弹出“打开”打开窗口，获取路径和文件名
{
	QString fileName = QFileDialog::getOpenFileName(parent,tr("Open"),".");
	if(fileName.isEmpty())
		return 0;

	return openFile(fileName,parent);
}

Editor *Editor::openFile(const QString &fileName,QWidget *parent)
{
	Editor *editor = new Editor(parent);
	if (editor->readFile(fileName)){
		editor->setCurrentFile(fileName);
		return editor;
	} else {
		delete editor;
		return 0;
	}
}

void Editor::closeEvent(QCloseEvent *event)//关闭事件发生时
{
	if (okToContinue()){
		event->accept();//接受关闭事件
	} else {
		event->ignore();//忽视关闭事件
	}
}

void Editor::documentWasModified()//设置窗口为被修改状态
{
	setWindowModified(true);
}

bool Editor::okToContinue()//是否继续
{
	if (document()->isModified())//判断文件是否被修改
	{
		int r = QMessageBox::warning(this,tr("MDI Editor"),
			tr("File %1 has been modified.\n"
			"Do you want to save your changes?").arg(strippedName(curFile)),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

		if (r == QMessageBox::Yes)
		{
			return save();
		} 
		else if(r == QMessageBox::Cancel)
		{
			return false;
		}
	}
	return true; 
}

bool Editor::saveFile(const QString &fileName)//保存文件
{
	if (writeFile(fileName))
	{
		setCurrentFile(fileName);
		return true;
	}
	else
	{
		return false;
	}
	
}

void Editor::setCurrentFile(const QString &fileName)//设置当前文件
{
	curFile = fileName;
	isUntitled = false;
	action->setText(strippedName(curFile));
	document()->setModified(false);
	setWindowTitle(strippedName(curFile) + "[*]");
	setWindowModified(false);
}

bool Editor::readFile(const QString &fileName)
{
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::warning(this,tr("MDI Editor"),
							tr("Cannot read file %1:\n&2.")
							.arg(file.fileName())
							.arg(file.errorString()));
		return false;
	}

	QTextStream in(&file);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	setPlainText(in.readAll());
	QApplication::restoreOverrideCursor();
	return true;
}

bool Editor::writeFile(const QString &fileName)
{
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::warning(this,tr("MDI Editor"),
			tr("Cannot write file %1:\n&2.")
			.arg(file.fileName())
			.arg(file.errorString()));
		return false;
	}

	QTextStream out(&file);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	out << toPlainText();
	QApplication::restoreOverrideCursor();
	return true;
}

QString Editor::strippedName(const QString &fullFileName)//从路径中剥离文件名
{
	return QFileInfo(fullFileName).fileName();
}



Editor::~Editor()
{

}