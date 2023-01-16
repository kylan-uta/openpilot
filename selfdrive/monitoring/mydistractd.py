#!/usr/bin/env python3

import time
import os
import subprocess
import cereal.messaging as messaging


def alert():
  subprocess.Popen(['./ui/tests/playsound', './assets/sounds/prompt_distracted.wav'], env={
    "LD_LIBRARY_PATH": "/system/lib64:" + os.environ['LD_LIBRARY_PATH'],
    'VOLUME': '1'
  })


def mydistractd_thread(sm=None, pm=None):
  if sm is None:
    sm = messaging.SubMaster(['driverMonitoringState'])

  last_dist_time = None
  while True:
    sm.update()

    driverMonitoringState = sm['driverMonitoringState']
    # print(driverMonitoringState)

    if not driverMonitoringState.isDistracted:
      last_dist_time = time.time()

    if last_dist_time and time.time() - last_dist_time > 3:
      alert()
      last_dist_time = time.time()

    
def main(sm=None, pm=None):
  mydistractd_thread(sm, pm)


if __name__ == '__main__':
  main()
