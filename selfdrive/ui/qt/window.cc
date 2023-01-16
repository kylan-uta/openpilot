#include "selfdrive/ui/qt/window.h"

#include <QFontDatabase>

#include "selfdrive/hardware/hw.h"

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
  main_layout = new QStackedLayout(this);
  main_layout->setMargin(0);

  driver_view = new DriverViewWindow(this);
  main_layout->addWidget(driver_view);
  QObject::connect(driver_view, &DriverViewWindow::openSettings, this, &MainWindow::openSettings);
  QObject::connect(driver_view, &DriverViewWindow::closeSettings, this, &MainWindow::closeSettings);

  // homeWindow = new HomeWindow(this);
  // main_layout->addWidget(homeWindow);
  // QObject::connect(homeWindow, &HomeWindow::openSettings, this, &MainWindow::openSettings);
  // QObject::connect(homeWindow, &HomeWindow::closeSettings, this, &MainWindow::closeSettings);

  settingsWindow = new SettingsWindow(this);
  main_layout->addWidget(settingsWindow);
  QObject::connect(settingsWindow, &SettingsWindow::closeSettings, this, &MainWindow::closeSettings);
  // QObject::connect(settingsWindow, &SettingsWindow::reviewTrainingGuide, [=]() {
    // onboardingWindow->showTrainingGuide();
    // main_layout->setCurrentWidget(onboardingWindow);
  // });
  // QObject::connect(settingsWindow, &SettingsWindow::showDriverView, [=] {
  //   homeWindow->showDriverView(true);
  // });
    
  alerts = new OnroadAlerts(this);
  alerts->setAttribute(Qt::WA_TransparentForMouseEvents, true);
  main_layout->addWidget(alerts);

  // setup stacking order
  alerts->raise();

  main_layout->setCurrentWidget(driver_view);

  // onboardingWindow = new OnboardingWindow(this);
  // main_layout->addWidget(onboardingWindow);
  // QObject::connect(onboardingWindow, &OnboardingWindow::onboardingDone, [=]() {
  //   main_layout->setCurrentWidget(homeWindow);
  // });
  // if (!onboardingWindow->completed()) {
  //   main_layout->setCurrentWidget(onboardingWindow);
  // }

  // QObject::connect(uiState(), &UIState::offroadTransition, [=](bool offroad) {
  //   if (!offroad) {
  //     closeSettings();
  //   }
  // });
  // QObject::connect(&device, &Device::interactiveTimout, [=]() {
  //   if (main_layout->currentWidget() == settingsWindow) {
  //     closeSettings();
  //   }
  // });

  // load fonts
  QFontDatabase::addApplicationFont("../assets/fonts/opensans_regular.ttf");
  QFontDatabase::addApplicationFont("../assets/fonts/opensans_bold.ttf");
  QFontDatabase::addApplicationFont("../assets/fonts/opensans_semibold.ttf");
  QFontDatabase::addApplicationFont("../assets/fonts/Inter-Black.ttf");
  QFontDatabase::addApplicationFont("../assets/fonts/Inter-Bold.ttf");
  QFontDatabase::addApplicationFont("../assets/fonts/Inter-ExtraBold.ttf");
  QFontDatabase::addApplicationFont("../assets/fonts/Inter-ExtraLight.ttf");
  QFontDatabase::addApplicationFont("../assets/fonts/Inter-Medium.ttf");
  QFontDatabase::addApplicationFont("../assets/fonts/Inter-Regular.ttf");
  QFontDatabase::addApplicationFont("../assets/fonts/Inter-SemiBold.ttf");
  QFontDatabase::addApplicationFont("../assets/fonts/Inter-Thin.ttf");

  // no outline to prevent the focus rectangle
  setStyleSheet(R"(
    * {
      font-family: Inter;
      outline: none;
    }
  )");
  setAttribute(Qt::WA_NoSystemBackground);
}

void MainWindow::openSettings() {
  main_layout->setCurrentWidget(settingsWindow);
}

void MainWindow::closeSettings() {
  main_layout->setCurrentWidget(driver_view);

  // if (uiState()->scene.started) {
  //   homeWindow->showSidebar(false);
  // }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
  const static QSet<QEvent::Type> evts({QEvent::MouseButtonPress, QEvent::MouseMove,
                                 QEvent::TouchBegin, QEvent::TouchUpdate, QEvent::TouchEnd});

  if (evts.contains(event->type())) {
    device.resetInteractiveTimout();
#ifdef QCOM
    // filter out touches while in android activity
    if (HardwareEon::launched_activity) {
      HardwareEon::check_activity();
      if (HardwareEon::launched_activity) {
        return true;
      }
    }
#endif
  }
  return false;
}

// ***** onroad widgets *****

// OnroadAlerts
// void OnroadAlerts::updateAlert(const Alert &a, const QColor &color) {
//   if (!alert.equal(a) || color != bg) {
//     alert = a;
//     bg = color;
//     update();
//   }
// }

// void OnroadAlerts::paintEvent(QPaintEvent *event) {
//   if (alert.size == cereal::ControlsState::AlertSize::NONE) {
//     return;
//   }
//   static std::map<cereal::ControlsState::AlertSize, const int> alert_sizes = {
//     {cereal::ControlsState::AlertSize::SMALL, 271},
//     {cereal::ControlsState::AlertSize::MID, 420},
//     {cereal::ControlsState::AlertSize::FULL, height()},
//   };
//   int h = alert_sizes[alert.size];
//   QRect r = QRect(0, height() - h, width(), h);

//   QPainter p(this);

//   // draw background + gradient
//   p.setPen(Qt::NoPen);
//   p.setCompositionMode(QPainter::CompositionMode_SourceOver);

//   p.setBrush(QBrush(bg));
//   p.drawRect(r);

//   QLinearGradient g(0, r.y(), 0, r.bottom());
//   g.setColorAt(0, QColor::fromRgbF(0, 0, 0, 0.05));
//   g.setColorAt(1, QColor::fromRgbF(0, 0, 0, 0.35));

//   p.setCompositionMode(QPainter::CompositionMode_DestinationOver);
//   p.setBrush(QBrush(g));
//   p.fillRect(r, g);
//   p.setCompositionMode(QPainter::CompositionMode_SourceOver);

//   // text
//   const QPoint c = r.center();
//   p.setPen(QColor(0xff, 0xff, 0xff));
//   p.setRenderHint(QPainter::TextAntialiasing);
//   if (alert.size == cereal::ControlsState::AlertSize::SMALL) {
//     configFont(p, "Open Sans", 74, "SemiBold");
//     p.drawText(r, Qt::AlignCenter, alert.text1);
//   } else if (alert.size == cereal::ControlsState::AlertSize::MID) {
//     configFont(p, "Open Sans", 88, "Bold");
//     p.drawText(QRect(0, c.y() - 125, width(), 150), Qt::AlignHCenter | Qt::AlignTop, alert.text1);
//     configFont(p, "Open Sans", 66, "Regular");
//     p.drawText(QRect(0, c.y() + 21, width(), 90), Qt::AlignHCenter, alert.text2);
//   } else if (alert.size == cereal::ControlsState::AlertSize::FULL) {
//     bool l = alert.text1.length() > 15;
//     configFont(p, "Open Sans", l ? 132 : 177, "Bold");
//     p.drawText(QRect(0, r.y() + (l ? 240 : 270), width(), 600), Qt::AlignHCenter | Qt::TextWordWrap, alert.text1);
//     configFont(p, "Open Sans", 88, "Regular");
//     p.drawText(QRect(0, r.height() - (l ? 361 : 420), width(), 300), Qt::AlignHCenter | Qt::TextWordWrap, alert.text2);
//   }
// }
