from __future__ import print_function
import time
from contextlib import closing
import socket, traceback
#from time import sleep
from select import select


def send(SourceIP):
  host = '192.168.1.101'   # Bind to all interfaces
  port = 10669
  time.sleep(1) 

  s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  #s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
  #s.bind((host, port))
  
  #for test use. delete after test
  #SourceIP = host
  message= "send to {0}".format(SourceIP).encode('utf-8')
  s.sendto(message, (SourceIP, port))
  print ("send packet to...",SourceIP)
  return

def main():
  host = ''   # Bind to all interfaces
  port = 10670
  
  s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
  s.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
  s.bind((host, port))

  while True:
      try:
          ins, outs, errs = select([s], [], [], 0)
          if ins:
              for i in ins:
                  message, address = i.recvfrom(8192)
                  print ("from", address)
                  send(address[0])
          #else:
          #    print "none..."
          #    sleep(1)
          
      except (KeyboardInterrupt, SystemExit):
          break
      except:
          traceback.print_exc()
          break


if __name__ == '__main__':
  main()
