#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QColorDialog>
#include <QMenu>

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = 0);
    ~SettingsWindow();

private slots:
    void on_buttonCancel_clicked();

	void on_buttonSave_clicked();

	void on_filterHilightColorButton_clicked();

	void filterHilightColor_changed(QColor);

	void loadSettings();

	void on_vproPathButton_clicked();

	private:
    Ui::SettingsWindow *ui;
	QColorDialog filterHilightColorDialog;
};

#endif // SETTINGSWINDOW_H
