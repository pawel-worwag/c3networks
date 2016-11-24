#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "SettingsWindow.h"
#include "AboutWindow.h"
#include "interfaces/AddEditIfWindow.h"
#include "interfaces/ChangeGroupWindow.h"
#include "interfaces/AddRangeWindow.h"
#include "ExportWindow.h"

#include "../database/InterfaceItem.h"
#include "groupsManager/GropusManagerWindow.h"

#include "ExternalAppWindow.h"

#include <QMessageBox>
#include <QPointer>

#include <QDebug>
#include <QHostAddress>

#include "global/LocalSettings.h"
#include "ifTypeManager/IfTypeManagerWindow.h"

#include <QFile>
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
	{
	ui->setupUi(this);
	this->ui->menuBar->insertAction(this->ui->menuBar->actions().at(2),this->ui->actionSettings);

	this->ui->statusBar->addWidget(&(this->dbNameLabel));

	this->tableModel = new IfTabeModel();
	connect(&ifDbTable,SIGNAL(prepareToModelReset()),this,SLOT(prepareToModelReset()));
	connect(this,SIGNAL(deleteInterfaces(QList<int>)),&ifDbTable,SLOT(deleteInterfaces(QList<int>)));

	this->tableProxy = new IfTableSortProxy();

	this->tableProxy->setSourceModel(this->tableModel);
	this->tableProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
	this->ui->ifTable->setModel(tableProxy);
	this->ui->ifTable->sortByColumn(0,Qt::AscendingOrder);
	this->tableModel->loadData();

	rightButtonMenu.insertAction(0,this->ui->actionChangeGroup);
	rightButtonMenu.addSeparator();
	rightButtonMenu.addAction(this->ui->actionPing);
	rightButtonMenu.addAction(this->ui->actionShowArp);
	rightButtonMenu.addAction(this->ui->actionDNS1);
	rightButtonMenu.addAction(this->ui->actionDNS2);
	rightButtonMenu.addAction(this->ui->actionVpro);

	this->ui->ifTable->setContextMenuPolicy(Qt::CustomContextMenu);
	this->ui->ifTable->resizeColumnToContents(IfTabeModel::IFTYPE);
	this->ui->ifTable->resizeColumnToContents(IfTabeModel::DHCPRESERVATION);
	this->ui->ifTableFilterBox->insertItem(0,tr("Ip Address"),IfTabeModel::IP);
	this->ui->ifTableFilterBox->insertItem(1,tr("Host Name"),IfTabeModel::NAME);
	this->ui->ifTableFilterBox->insertItem(2,tr("Interface Type"),IfTabeModel::IFTYPE);
	this->ui->ifTableFilterBox->insertItem(3,tr("MAC Address"),IfTabeModel::MAC);
	this->ui->ifTableFilterBox->insertItem(4,tr("User Name"),IfTabeModel::USERNAME);
	this->ui->ifTableFilterBox->insertItem(5,tr("Location"),IfTabeModel::LOCATION);
	this->ui->ifTableFilterBox->insertItem(6,tr("Switch port"),IfTabeModel::SWITCHPORT);
	this->ui->ifTableFilterBox->insertItem(7,tr("Domain"),IfTabeModel::HOSTDOMAIN);
	this->ui->ifTableFilterBox->insertItem(8,tr("Description"),IfTabeModel::DESCRIPTION);

	this->ui->tabWidget->setCurrentIndex(0);

	loadSettings();
	}

MainWindow::~MainWindow()
	{
	delete ui;
	}

void MainWindow::on_ifTableFilterEdit_textChanged(const QString &arg1)
	{
	//this->tableProxy->setFilterFixedString(".*"+arg1+".*");
	this->tableProxy->setFilterRegExp("^.*"+arg1+".*$");
	this->ui->ifTable->reset();
	}

void MainWindow::on_buttonRefresh_clicked()
	{
	this->tableModel->loadData();
	}

void MainWindow::on_pushButton_clicked()
	{
	AddEditIfWindow *w = new AddEditIfWindow(this);
	w->setAttribute(Qt::WA_DeleteOnClose);
	connect(w,SIGNAL(addInterface(quint32,quint32,QString,QString,QString,QString,QString,bool,int,QString,bool,int,QString)),
			&ifDbTable,SLOT(addInterface(quint32,quint32,QString,QString,QString,QString,QString,bool,int,QString,bool,int,QString)));
	w->addInterface();
	}

void MainWindow::on_buttonEditHost_clicked()
	{
	QList<QModelIndex> list = this->ui->ifTable->selectionModel()->selectedRows();
	qDebug()<<list;

	if(list.count()==1)
		{
		QPointer<InterfaceItem> item = tableModel->getInterfaceItem(tableProxy->mapToSource(list.first()));


		if(!item.isNull())
			{
			AddEditIfWindow *w = new AddEditIfWindow(this);
			w->setAttribute(Qt::WA_DeleteOnClose);
			connect(w,SIGNAL(updateInterface(int,quint32,quint32,QString,QString,QString,QString,QString,bool,int,QString,bool,int,QString)),
					&ifDbTable,SLOT(updateInterface(int,quint32,quint32,QString,QString,QString,QString,QString,bool,int,QString,bool,int,QString)));
			w->updateInterface(item->getId());
			}
		}
	}

void MainWindow::on_ifTable_doubleClicked(const QModelIndex &index)
	{
	Q_UNUSED(index)
	on_buttonEditHost_clicked();
	}

void MainWindow::prepareToModelReset()
	{
	this->tableModel->loadData();
	}

void MainWindow::setDbName(QString name)
	{
	this->dbNameLabel.setText(tr("Database: ")+name);
	}

void MainWindow::on_actionAbout_Application_triggered()
	{
	AboutWindow *w = new AboutWindow(this);
	w->setAttribute(Qt::WA_DeleteOnClose);
	w->show();
	}

void MainWindow::on_actionAbout_QT_triggered()
	{
	QApplication::aboutQt();
	}

void MainWindow::on_actionSettings_triggered()
	{
	SettingsWindow *w = new SettingsWindow(this);
	w->setAttribute(Qt::WA_DeleteOnClose);
	w->show();
	}

void MainWindow::on_buttonRemoveHost_clicked()
	{
	QList<QModelIndex> list = this->ui->ifTable->selectionModel()->selectedRows();
	QList<int> idx;

	if(list.count()>0)
		{
		QMessageBox box;
		box.setWindowTitle(tr("Delete"));
		box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		box.setDefaultButton(QMessageBox::No);
		box.setButtonText(QMessageBox::No,tr("No"));
		box.setButtonText(QMessageBox::Yes,tr("Yes"));
		box.setIcon(QMessageBox::Critical);
		box.setText(tr("Would you like to delete selected records?"));
		if(box.exec() == QMessageBox::Yes)
			{

			for(int i=0; i< list.count();i++)
				{
				QPointer<InterfaceItem> item = tableModel->getInterfaceItem(tableProxy->mapToSource(list.at(i)));
				if(!item.isNull())
					{
					idx.append(item->getId());
					}
				}
			emit deleteInterfaces(idx);
			}
		}
	}

void MainWindow::on_ifTable_customContextMenuRequested(const QPoint &pos)
	{
	this->rightButtonMenu.popup(this->ui->ifTable->viewport()->mapToGlobal(pos));
	}

void MainWindow::on_actionChangeGroup_triggered()
	{
	QList<QModelIndex> list = this->ui->ifTable->selectionModel()->selectedRows();
	QList<int> idx;

	if(list.count()>0)
		{
		for(int i=0; i< list.count();i++)
			{
			QPointer<InterfaceItem> item = tableModel->getInterfaceItem(tableProxy->mapToSource(list.at(i)));
			if(!item.isNull())
				{
				idx.append(item->getId());
				}
			}

		ChangeGroupWindow *w = new ChangeGroupWindow(this);
		w->setAttribute(Qt::WA_DeleteOnClose);
		connect(w,SIGNAL(changeGroup(QList<int>,int)),&ifDbTable,SLOT(changeGroup(QList<int>,int)));
		w->setIdxList(idx);
		w->show();
		}
	}

void MainWindow::on_pushAddRange_clicked()
	{
	AddRangeWindow *w = new AddRangeWindow(this);
	w->setAttribute(Qt::WA_DeleteOnClose);
	connect(w,SIGNAL(addInterfaces(quint32,quint32,quint32,int)),&ifDbTable,SLOT(addInterfaces(quint32,quint32,quint32,int)));
	w->show();
	}

void MainWindow::on_actionExport_triggered()
	{
	ExportWindow *w = new ExportWindow(this);
	w->setAttribute(Qt::WA_DeleteOnClose);
	connect(w,SIGNAL(exportAllToCSV(QString,bool,bool,bool,bool,bool,bool,bool,bool,bool,bool,bool,bool,QString)),
			&ifDbTable,SLOT(exportAllToCSV(QString,bool,bool,bool,bool,bool,bool,bool,bool,bool,bool,bool,bool,QString)));
	w->show();
	}

void MainWindow::on_ifTableFilterModeButton_toggled(bool checked)
	{
	this->tableProxy->setAllColumnsFilter(checked);
	this->ui->ifTableFilterBox->setEnabled(!checked);
	this->tableProxy->setFilterRegExp("^.*"+this->ui->ifTableFilterEdit->text()+".*$");
	this->ui->ifTable->reset();
	LocalSettings s;
	if(checked)
		{
		s.setValue("FilterMethod","AllColumns");
		}
	else
		{
		s.setValue("FilterMethod",this->ui->ifTableFilterBox->currentIndex());
		}
	}

void MainWindow::on_ifTableFilterBox_activated(int index)
	{
	this->tableProxy->setFilterKeyColumn(this->ui->ifTableFilterBox->currentData().toInt());
	this->tableProxy->setFilterRegExp("^.*"+this->ui->ifTableFilterEdit->text()+".*$");
	LocalSettings s;
	s.setValue("FilterMethod",index);
	}

void MainWindow::on_actionPing_triggered()
	{
	QList<QModelIndex> list = this->ui->ifTable->selectionModel()->selectedRows();
	if(list.count()>0)
		{
		for(int i=0; i< list.count();i++)
			{
			QPointer<InterfaceItem> item = tableModel->getInterfaceItem(tableProxy->mapToSource(list.at(i)));
			if(!item.isNull())
				{
				QStringList params;
				params << QHostAddress(item->getIpAddress()).toString();
				executeApp("ping",params);
				}
			}
		}
	}

void MainWindow::executeApp(QString app, QStringList params)
	{
	ExternalAppWindow *w = new ExternalAppWindow();
	w->setAttribute(Qt::WA_DeleteOnClose);
	w->executeApp(app,params);
	}

void MainWindow::loadSettings()
	{
	LocalSettings s;
	if(s.value("rememberFilterMode").toBool())
		{
		QString fm = s.value("FilterMethod").toString();
		if(fm == "AllColumns")
			{
			this->ui->ifTableFilterModeButton->setChecked(true);
			}
		else
			{
			this->ui->ifTableFilterBox->setCurrentIndex(fm.toInt());
			this->ui->ifTableFilterModeButton->setChecked(false);
			}
		}
	}

void MainWindow::on_actionShowArp_triggered()
	{
	QList<QModelIndex> list = this->ui->ifTable->selectionModel()->selectedRows();
	if(list.count()>0)
		{
		for(int i=0; i< list.count();i++)
			{
			QPointer<InterfaceItem> item = tableModel->getInterfaceItem(tableProxy->mapToSource(list.at(i)));
			if(!item.isNull())
				{
				QStringList params;
				params <<"-a";
				params << QHostAddress(item->getIpAddress()).toString();
				executeApp("arp",params);
				}
			}
		}
	}

void MainWindow::on_actionDNS1_triggered()
	{
	QList<QModelIndex> list = this->ui->ifTable->selectionModel()->selectedRows();
	if(list.count()>0)
		{
		for(int i=0; i< list.count();i++)
			{
			QPointer<InterfaceItem> item = tableModel->getInterfaceItem(tableProxy->mapToSource(list.at(i)));
			if(!item.isNull() && !item->getName().trimmed().isEmpty())
				{
				QStringList params;
				params << item->getName();
				executeApp("nslookup",params);
				}
			}
		}
	}

void MainWindow::on_actionDNS2_triggered()
	{
	QList<QModelIndex> list = this->ui->ifTable->selectionModel()->selectedRows();
	if(list.count()>0)
		{
		for(int i=0; i< list.count();i++)
			{
			QPointer<InterfaceItem> item = tableModel->getInterfaceItem(tableProxy->mapToSource(list.at(i)));
			if(!item.isNull() && !item->getName().trimmed().isEmpty())
				{
				QStringList params;
				params << QHostAddress(item->getIpAddress()).toString();
				executeApp("nslookup",params);
				}
			}
		}
	}

void MainWindow::on_tabWidget_currentChanged(int index)
	{
	if(index!=0)
		{
		this->ui->filterWidget->hide();
		}
	else
		{
		this->ui->filterWidget->show();
		}
	}

void MainWindow::on_actionDevice_Groups_triggered()
	{
	GropusManagerWindow *w = new GropusManagerWindow(this);
	w->setAttribute(Qt::WA_DeleteOnClose);
	w->show();
	}

void MainWindow::on_actionInterface_Types_triggered()
	{
	IfTypeManagerWindow *w = new IfTypeManagerWindow(this);
	w->setAttribute(Qt::WA_DeleteOnClose);
	connect(w,SIGNAL(refreshDatabase()),this->tableModel,SLOT(loadData()));
	w->show();

	}

void MainWindow::on_actionVpro_triggered()
	{
	QList<QModelIndex> list = this->ui->ifTable->selectionModel()->selectedRows();
	if(list.count()>0)
		{
		for(int i=0; i< list.count();i++)
			{
			QPointer<InterfaceItem> item = tableModel->getInterfaceItem(tableProxy->mapToSource(list.at(i)));
			if(!item.isNull() && !item->getName().trimmed().isEmpty())
				{
				LocalSettings s;
				QString path = s.value("vproPath").toString()+"/"+item->getName()+".vnc+";
				if(QFile(path).exists())
					{
					QDesktopServices::openUrl(QUrl::fromLocalFile(path));
					}
				else
					{
					QMessageBox box(this);
					box.setWindowTitle(tr("Execute"));
					box.setStandardButtons(QMessageBox::Ok);
					box.setButtonText(QMessageBox::Ok,tr("Ok"));
					box.setIcon(QMessageBox::Warning);
					box.setText(tr("File not found"));
					box.exec();
					}
				}
			}
		}
	}
