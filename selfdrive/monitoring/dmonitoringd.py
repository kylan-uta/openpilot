#!/usr/bin/env python3
import gc

import cereal.messaging as messaging
from cereal import car, log
from common.params import Params
from common.realtime import set_realtime_priority
from selfdrive.controls.lib.events import Events
from selfdrive.locationd.calibrationd import Calibration
from selfdrive.monitoring.driver_monitor import DriverStatus

import os
import subprocess
import time

def alert(pm, name):
  # subprocess.Popen(['/data/openpilot/selfdrive/ui/playsound', '/data/openpilot/selfdrive/assets/sounds/' + name + '.wav'], env={
  #   "LD_LIBRARY_PATH": "/system/lib64:" + os.environ['LD_LIBRARY_PATH'],
  #   'VOLUME': '1'
  # })
  # os.system('LD_LIBRARY_PATH=/system/lib64:$LD_LIBRARY_PATH /data/openpilot/selfdrive/ui/playsound /data/openpilot/selfdrive/assets/sounds/' + name + '.wav')
  print('!!!!! ' + name)
  dat = messaging.new_message('controlsState')
  dat.controlsState.alertSound = car.CarControl.HUDControl.AudibleAlert.promptDistracted
  dat.controlsState.alertType = '1'
  dat.controlsState.alertText1 = "Testing Sounds"
  dat.controlsState.alertText2 = "playing ..."
  dat.controlsState.alertSize = log.ControlsState.AlertSize.mid
  pm.send('controlsState', dat)


def dmonitoringd_thread(sm=None, pm=None):
  last_dist_time = None
  cnt = 0
  gc.disable()
  set_realtime_priority(2)

  if pm is None:
    pm = messaging.PubMaster(['driverMonitoringState', 'controlsState'])

  if sm is None:
    sm = messaging.SubMaster(['driverState', 'liveCalibration', 'carState', 'controlsState', 'modelV2'], poll=['driverState'])

  driver_status = DriverStatus(rhd=Params().get_bool("IsRHD"))

  sm['liveCalibration'].calStatus = Calibration.INVALID
  sm['liveCalibration'].rpyCalib = [0, 0, 0]
  sm['carState'].buttonEvents = []
  sm['carState'].standstill = True

  v_cruise_last = 0
  driver_engaged = False
  ctrl_active = True
  alert_type = None

  # 10Hz <- dmonitoringmodeld
  while True:
    sm.update()

    if not sm.updated['driverState']:
      continue

    # Get interaction
    if sm.updated['carState']:
      v_cruise = sm['carState'].cruiseState.speed
      driver_engaged = len(sm['carState'].buttonEvents) > 0 or \
                        v_cruise != v_cruise_last or \
                        sm['carState'].steeringPressed or \
                        sm['carState'].gasPressed
      v_cruise_last = v_cruise

    if sm.updated['modelV2']:
      driver_status.set_policy(sm['modelV2'], sm['carState'].vEgo)

    # Get data from dmonitoringmodeld
    events = Events()
    driver_status.get_pose(sm['driverState'], sm['liveCalibration'].rpyCalib, sm['carState'].vEgo, sm['controlsState'].enabled)

    # Block engaging after max number of distrations
    if driver_status.terminal_alert_cnt >= driver_status.settings._MAX_TERMINAL_ALERTS or \
       driver_status.terminal_time >= driver_status.settings._MAX_TERMINAL_DURATION:
      events.add(car.CarEvent.EventName.tooDistracted)      

    # Update events from driver state
    driver_status.update(events, False, ctrl_active, False)
    # driver_status.update(events, driver_engaged, sm['controlsState'].enabled, sm['carState'].standstill)

    # build driverMonitoringState packet
    dat = messaging.new_message('driverMonitoringState')
    dat.driverMonitoringState = {
      "events": events.to_msg(),
      "faceDetected": driver_status.face_detected,
      "isDistracted": driver_status.driver_distracted,
      "awarenessStatus": driver_status.awareness,
      "posePitchOffset": driver_status.pose.pitch_offseter.filtered_stat.mean(),
      "posePitchValidCount": driver_status.pose.pitch_offseter.filtered_stat.n,
      "poseYawOffset": driver_status.pose.yaw_offseter.filtered_stat.mean(),
      "poseYawValidCount": driver_status.pose.yaw_offseter.filtered_stat.n,
      "stepChange": driver_status.step_change,
      "awarenessActive": driver_status.awareness_active,
      "awarenessPassive": driver_status.awareness_passive,
      "isLowStd": driver_status.pose.low_std,
      "hiStdCount": driver_status.hi_stds,
      "isActiveMode": driver_status.active_monitoring_mode,
    }
    pm.send('driverMonitoringState', dat)
    
    eve = events.to_msg()
    if not len(eve):
      cur_alert_type = 'none'
    else:
      cur_alert_type = eve[0].name

    if alert_type != cur_alert_type:
      alert_type = cur_alert_type
      cnt = 0
      if alert_type == 'tooDistracted':
        alert(pm, 'warning_immediate')
        # alert(pm, 'li_ke_ting_che_xiu_xi')
        driver_status.terminal_alert_cnt = 0
        driver_status.terminal_time = 0
        # alert(pm, 'zh_distracted')
      elif alert_type == 'promptDriverDistracted':
        alert(pm, 'prompt_distracted')
        # alert(pm, 'qing_zhu_yi_an_quan')
        # alert(pm, 'zh_distracted')
      elif alert_type == 'driverDistracted':
        driver_status.awareness = 1.
        driver_status.awareness_active = 1.
        driver_status.awareness_passive = 1.
        alert(pm, 'warning_soft')
        # alert(pm, 'zh_distracted')
    else:
      cnt = cnt + 1

    print(alert_type, cnt)
    print(driver_status.awareness)
    print('tac', driver_status.terminal_alert_cnt, 'tt', driver_status.terminal_time)
    print('-' * 20)

    # if last_dist_time and time.time() - last_dist_time > 3:
    #   if len(eve) == 2:
    #     alert('refuse')
    #     driver_status.awareness = 1.
    #     driver_status.awareness_active = 1.
    #     driver_status.awareness_passive = 1.
    #   else:
    #     alert('prompt_distracted')
    #   last_dist_time = time.time()


def main(sm=None, pm=None):
  dmonitoringd_thread(sm, pm)


if __name__ == '__main__':
  main()


# installation guide：
# scp D:\openpilot\selfdrive\ui\tests\playsound.cc root@192.168.1.74:/data/openpilot/selfdrive/ui
# scp D:\openpilot\selfdrive\ui\qt\offroad\driverview.cc D:\openpilot\selfdrive\ui\qt\offroad\driverview.h root@192.168.1.74:/data/openpilot/selfdrive/ui/qt/offroad
# scp D:\openpilot\selfdrive\ui\qt\window.cc D:\openpilot\selfdrive\ui\qt\window.h root@192.168.1.74:/data/openpilot/selfdrive/ui/qt
# scp D:\openpilot\selfdrive\ui\SConscript root@192.168.1.74:/data/openpilot/selfdrive/ui
# scp D:\openpilot\selfdrive\monitoring\dmonitoringd.py root@192.168.1.74:/data/openpilot/selfdrive/monitoring
# scp D:\openpilot\selfdrive\assets\sounds\zh_stop.wav D:\openpilot\selfdrive\assets\sounds\zh_attention.wav D:\openpilot\selfdrive\assets\sounds\zh_distracted.wav root@192.168.1.74:/data/openpilot/selfdrive/assets/sounds/
# scp D:\openpilot\selfdrive\thermald\thermald.py root@192.168.1.74:/data/openpilot/selfdrive/thermald

# export HTTPS_PROXY=http://192.168.1.20:1081;export HTTP_PROXY=http://192.168.1.20:1081