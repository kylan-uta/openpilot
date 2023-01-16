#pragma once

#include <QStackedLayout>
#include <QWidget>

#include "selfdrive/ui/qt/home.h"
#include "selfdrive/ui/qt/offroad/onboarding.h"
#include "selfdrive/ui/qt/offroad/settings.h"
#include "selfdrive/ui/ui.h"


// class OnroadAlerts : public QWidget {
//   Q_OBJECT

// public:
//   OnroadAlerts(QWidget *parent = 0) : QWidget(parent) {};
//   void updateAlert(const Alert &a, const QColor &color);

// protected:
//   void paintEvent(QPaintEvent*) override;

// private:
//   QColor bg;
//   Alert alert = {};
// };

class MainWindow : public QWidget {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);

private:
  bool eventFilter(QObject *obj, QEvent *event) override;
  void openSettings();
  void closeSettings();

  Device device;

  QStackedLayout *main_layout;
  OnroadAlerts *alerts;
  SettingsWindow *settingsWindow;
  // HomeWindow *homeWindow;
  // OnboardingWindow *onboardingWindow;
  DriverViewWindow *driver_view;
};
