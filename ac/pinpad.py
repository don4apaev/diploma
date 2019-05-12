#!/usr/bin/python3
# -*- coding: utf-8 -*-
import RPi.GPIO as GPIO
import threading,sys,time,socket,requests, logging, os, sqlite3, serial, pygame
from http.server import HTTPServer, CGIHTTPRequestHandler

from umodbus            import conf
from umodbus.client     import tcp
''' GUI modules import'''
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (QWidget, QPushButton, QVBoxLayout,QHBoxLayout, QApplication, QFrame, QLabel)
from PyQt5.QtGui import (QColor)

serv_pin         = (('0','7','0','8'),('2','0','1','7'))
tmp_pin          = ('1','2','3','4')
tmp_2_pin        = ('4','3','2','1')

Threat_Flags = [0]

serv_pin_count    = 0 #counter of server pin code parts for checking 
door_pin        = 24 #BCM raspberry GPIO door reley pin
reed_pin        = 23

lastrowid       = 0

hall_id          = 30
api_url          = 'http://10.11.25.5:4000'

serv_addr        = '10.11.13.40'
gpio_lock        = threading.Lock()
ind_serial       = serial.Serial()
ind_serial.baudrate  = 19200
ind_serial.port      = "/dev/ttyUSB0"

def openDoor(ip):
    global Threat_Flags
    try:#open door
        if not Threat_Flags[0]:
            try:
                sock_modbus = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock_modbus.connect(('10.11.13.'+ip, 502))
                str_message = tcp.write_single_coil(slave_id=1,address=100, value=1)
                list_response = tcp.send_message(str_message,sock_modbus)
            except  socket.error as e:
                print('except')
                QMessageBox(QMessageBox.Critical,"Error:\n",str(e)).exec()
            else:
                pygame.mixer.music.load("fb.mp3")
                pygame.mixer.music.play(5)
                createRecord()
                GameThread(ip).start()
                setDoor(0)
                SleepThread(10,setDoor,1).start()
            finally:
                sock_modbus.close()
    except:#open door
        answerMessage = '{} >> unenable to start game'.format(
            time.strftime('%H:%M:%S|%y.%m.%d',time.localtime()))
        logging.warning(answerMessage)

    else:#open door
        answerMessage = '{} >> start game'.format(
            time.strftime('%H:%M:%S|%y.%m.%d',time.localtime())) 
        logging.info(answerMessage)
                
def setDoor(seting):
    global door_pin, gpio_lock, Threat_Flags
    gpio_lock.acquire()
    try:#open door
        if not Threat_Flags[0]:
            if seting == 1:
                GPIO.setmode(GPIO.BCM)
                GPIO.setup(door_pin, GPIO.OUT, initial=GPIO.HIGH)
                pass
            elif seting == 0:
                GPIO.setmode(GPIO.BCM)
                GPIO.setup(door_pin, GPIO.OUT, initial=GPIO.LOW)
                pass
    except:#open door
        answerMessage = '{} >> unenable to toggle door'.format(
            time.strftime('%H:%M:%S|%y.%m.%d',time.localtime()))
        logging.warning(answerMessage)
    else:#open door
        answerMessage = '{} >> set door status {}'.format(
            time.strftime('%H:%M:%S|%y.%m.%d',time.localtime()),str(seting))
        logging.info(answerMessage)
    finally:#open door
        gpio_lock.release()
   
def pinCheck(playerinsert):
        global api_url, hall_id
        if tuple(playerinsert) == tmp_pin:
            return 1
        elif tuple(playerinsert) == tmp_2_pin:
            return 2
        r = requests.get('{}/halls/checkAccess?key=3471e1b95646&code={}&hallId={}'
                         .format(api_url, ''.join(playerinsert), hall_id))
        if r.status_code == 200:
            answerMessage = '{} >> Recive right pin {}'.format(
                time.strftime('%H:%M:%S|%y.%m.%d',time.localtime()),''.join(playerinsert))
            logging.info(answerMessage)
            return 1
        else:#if r.status_code == 200
            return 0

def servPinCheck(playerinsert):
        global serv_pin_count, serv_pin
        for cha,tmpcha in zip(playerinsert,serv_pin[serv_pin_count]):
            if cha !=tmpcha:
                serv_pin_count = 0
                return 0
        serv_pin_count += 1
        if serv_pin_count == 2:
            serv_pin_count = 0
            answerMessage = '{} >> Recive right server pin'.format(time.strftime('%H:%M:%S|%y.%m.%d',time.localtime()))
            logging.info(answerMessage)
            return 1
        else: #if serv_pin_count == 2
            return 0

class PinPad(QWidget):

    def __init__(self, PinCh, ServPinCh, OpenD, SetD,LCDclearDelay=10):
        super().__init__()
        self.insCount = 0 #player insert count
        self.waitFlag = 0 #flag for clear LCD or input collision preventing
        self.doorStat = 1 #door close status
        self.playerIns = [' ',' ',' ',' '] # player pin insert list
        self.clearDelay = LCDclearDelay #LCD show delay
        self.pinCheck = PinCh #check ticket server for pin
        self.servPinCheck = ServPinCh #check if pin is server pin
        self.openDoor = OpenD #open door and start game
        self.setDoor = SetD #toggle door status
        self.initUI()
        
    def initUI(self):
        self.topsquare = QFrame(self)
        self.topsquare.setGeometry(0, 0, 800, 1200)
        #self.topsquare.setStyleSheet("QWidget { background-color: #AEECF0 }")
        #self.topsquare.setStyleSheet("QWidget { background-color: rgb(35,58,58) }")

        #add lcds to prog
        '''
        0/O, 1, 2, 3, 4, 5/S, 6, 7, 8, 9/g,
        minus, decimal point, A, B, C, D, E,
        F, h, H, L, o, P, r, u, U, Y, colon,
        degree sign (which is specified as
        single quote in the string) and space
        '''
        #self.lcds   = [QLCDNumber(self),QLCDNumber(self),
        #               QLCDNumber(self),QLCDNumber(self)]
        self.lcds   = [QLabel(self),QLabel(self),
                       QLabel(self),QLabel(self)]


        Ubox        = QHBoxLayout()
    

        self.verticalLayoutR = QVBoxLayout()
        self.verticalLayoutR.setSpacing(0)
        self.exitFrame = QFrame(self)
        self.exitFrame.setStyleSheet("background-color: #AEECF0;")
        #self.exitFrame.setFrameShape(QFrame.StyledPanel)
        #self.exitFrame.setFrameShadow(QFrame.Raised)
        self.exitverticalLayout = QHBoxLayout(self.exitFrame)
        self.exitverticalLayout.setSpacing(0)
         
        for lcd in self.lcds:
            #lcd.display(' ')
            lcd.setStyleSheet("background-color: rgb(35,58,58); font-size:50px;font: bold 50px; color: #AEECF0")
            lcd.setAlignment(Qt.AlignCenter)
            lcd.setText(' ')
            Ubox.addWidget(lcd)

            self.exitverticalLayout.addWidget(lcd)

        self.topLbl      = QLabel(self.exitFrame)
        self.topLbl.setWordWrap(True)
        self.topLbl.setAlignment(Qt.AlignCenter)
        self.topLbl.setGeometry(9, 9, 760, 280)
        self.messageShowEnterPin()

        self.verticalLayoutR.addWidget(self.exitFrame)

        #add buttons to prog
        names       = [['1', '2', '3'], ['4', '5', '6'],
                       ['7', '8', '9'], ['Clear', '0', 'OK']]
        Uboxes      = [QHBoxLayout(),QHBoxLayout(),
                       QHBoxLayout(),QHBoxLayout()]

        #set output 
        Vbox        = QVBoxLayout()
        Vbox.setSpacing(0)
        Vbox.setContentsMargins(-1, -1, -1, -1)
        Vbox.addLayout(self.verticalLayoutR)

        #for box in Uboxes:
        #    Vbox.addLayout(box)

        for box, name in zip(Uboxes, names):
            nLayoutR = QVBoxLayout()
            nLayoutR.setContentsMargins(-1, -1, -1, -1)
            nLayoutR.setSpacing(0)
            nFrame = QFrame(self)
            nFrame.setStyleSheet("background-color: #AEECF0;")
            #nFrame.setFrameShape(QFrame.StyledPanel)
            #nFrame.setFrameShadow(QFrame.Raised)
            box = QHBoxLayout(nFrame)

            for n in name:
                button = QPushButton(n,self)
 
                palette = button.palette()
                palette.setColor(button.backgroundRole(),QColor(35,58,58))
                button.setPalette(palette)
                button.setStyleSheet("background-color: #233A3A; font-size:70px;font: bold 90px; color: #AEECF0")
                button.setAutoFillBackground(True)
                #button.setFlat(True)                

                button.setMinimumHeight(220)
                if n == 'Clear':
                    button.clicked.connect(self.clearClicked)
                elif n == 'OK':
                    button.clicked.connect(self.okClicked)
                else:
                    button.clicked.connect(self.buttonClicked)

                #button.setAutoFillBackground(True)

                box.addWidget(button)

            nLayoutR.addWidget(nFrame)
            Vbox.addLayout(nLayoutR)

        self.setStyleSheet("QPushButton{color:#AEECF0;font: bold 70px; }")
        self.setStyleSheet("QVBoxLayout { background-color: #AEECF0 }")
        #self.setStyleSheet("background: rgb(35,58,58)")
        self.setLayout(Vbox)
        self.showFullScreen()
        
        #self.show()

    def messageShowEnterPin(self):
        self.topLbl.setStyleSheet("background-color: rgb(35,58,58); font-size:50px;font: bold 50px; color: #AEECF0")
        self.topLbl.setText("ВВЕДИТЕ КОД ДОСТУПА И  НАЖМИТЕ ОК")
        self.topLbl.show()

    def messageShowErrorPin(self):
        self.topLbl.setStyleSheet("background-color: #FF0000; font-size:50px;font: bold 50px; color: #000000")
        self.topLbl.setText("НЕВЕРНЫЙ КОД")
        self.topLbl.show()
        
    def keyPressEvent(self, e):#escape key 
        if e.key() == Qt.Key_Escape:
            self.close()

    def messageShow(self, lcdmsg):#show msg on lcd
        self.topLbl.setStyleSheet("background-color: #00FF00; font-size:50px;font: bold 50px; color: #000000")
        self.topLbl.setText(lcdmsg)
        self.topLbl.show()
            
    def buttonClicked(self):#number button pressed
        if not self.waitFlag:
            sender = self.sender()
            if self.insCount < 4:
                #self.lcds[self.insCount].display(sender.text())
                self.lcds[self.insCount].setText(sender.text())
                self.playerIns[self.insCount] = sender.text()
                self.insCount +=1
                self.topLbl.hide()
            elif self.servPinCheck(self.playerIns):
                self.doorStat = 1 - self.doorStat
                shownmsg = "ЗАКРЫТО" if self.doorStat else "ОТКРЫТО"
                self.messageShow(shownmsg) #show status on LCD and block keys
                self.setDoor(self.doorStat)
                #clear LCD and unblock keys after clearDelay sec
                SleepThread(self.clearDelay,self.clearClicked,fncArg=1).start()
            else:
                self.clearClicked()
                #self.lcds[self.insCount].display(sender.text())
                    
    def clearClicked(self, flagUnset=0):#clear lcds; flagUnset for waiting flag
        if flagUnset: self.waitFlag=0   
        if not self.waitFlag:
            self.insCount = 4
            for lcd in self.lcds:
                self.insCount -=1
                #lcd.display(' ')
                lcd.setText(' ')
                self.playerIns[self.insCount] = ''                
            self.messageShowEnterPin()

    def okClicked(self) :#check pin
        if not self.waitFlag:
            if self.insCount ==4:
                tmp_res = self.pinCheck(self.playerIns)
                if tmp_res:
                    self.messageShow("ОТКРОЙТЕ ДВЕРЬ И НАЧНИТЕ ИГРУ") #show status on LCD and block keys
                    self.openDoor('104' if tmp_res ==1 else '105')
                    #clear LCD and unblock keys after clearDelay sec
                    SleepThread(self.clearDelay,self.clearClicked,fncArg=1).start()
                else:
                    self.messageShowErrorPin()
                    SleepThread(self.clearDelay,self.clearClicked,fncArg=1).start()

                    #self.clearClicked()
            else:#if self.insCount ==4
                self.clearClicked()     
                   
class SleepThread(threading.Thread):
    
    def __init__(self, waitTime,runFnc,fncArg=None):
        threading.Thread.__init__(self)
        self.waitTime = waitTime
        self.runFnc = runFnc
        self.fncArg = fncArg
    
    def run(self):
        time.sleep(self.waitTime)
        try:
            if self.fncArg is None:self.runFnc() 
            else: self.runFnc(self.fncArg)
        finally:
            return
        
class SocketThread(threading.Thread):
    def __init__(self,srvAddr,flglnk):
        threading.Thread.__init__(self)
        self.daemon = True
        self.server_address = srvAddr
        self.flag_link = flglnk
        self.previos_pin_stat = 0
        
    def run(self):
        sock = socket.socket()
        sock.bind(("", 2017))
        sock.listen(1)
        try:#fire socket
            while True:
                conn, addr = sock.accept()
                if addr[0] != self.server_address:
                    print('wrong server')
                    conn.send("connection error. wrong server".encode("utf-8"))
                    conn.close()
                    continue
                data = ''
                try:#recive data
                    data = conn.recv(1024)
                except socket.error as e:#recive data
                    answerMessage = '{} >> socket connection error {}'.format(
                        time.strftime('%H:%M:%S|%y.%m.%d',time.localtime()),e.args[0])
                    logging.warning(answerMessage)
                else:#recive data
                    if data == b'FIRE':#data parse
                        print('FIRE')
                        global door_pin, player_pin, gpio_lock
                        gpio_lock.acquire()
                        try:#anlock door
                            self.flag_link[0] = 1
                            pygame.mixer.music.stop()
                            GPIO.setmode(GPIO.BCM)
                            GPIO.setup((door_pin), GPIO.OUT)
                            self.previos_pin_stat = GPIO.input(door_pin)
                            GPIO.output((door_pin), GPIO.LOW)
                            sock_modbus = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                            sock_modbus.connect(('10.11.13.104', 502))
                            str_message = tcp.write_single_coil(slave_id=1,address=100, value=4)
                            list_response = tcp.send_message(str_message,sock_modbus)
                            
                        except:#anlock door
                            conn.send("Error while unblock doors".encode("utf-8"))
                            answerMessage = '{} >> Error while unblock doors'.format(
                                time.strftime('%H:%M:%S|%y.%m.%d',time.localtime()))
                            logging.warning(answerMessage)
                        else:#anlock door
                            conn.send("Unblock doors".encode("utf-8"))
                            answerMessage = '{} >> Unblock door by fire alarm'.format(
                                time.strftime('%H:%M:%S|%y.%m.%d',time.localtime()))
                            logging.info(answerMessage)
                        finally:#anlock door
                            gpio_lock.release()
                            sock_modbus.close()
                    elif data == b'ALLOW':#data parse
                        print('ALLOW')
                        global door_pin, player_pin, gpio_lock
                        gpio_lock.acquire()
                        try:#allow blocking
                            self.flag_link[0] = 0
                            GPIO.setmode(GPIO.BCM)
                            GPIO.setup((door_pin), GPIO.OUT)
                            GPIO.output((door_pin), self.previos_pin_stat)
                        except:#allow blocking
                            conn.send("Error while unlock AC".encode("utf-8"))
                            answerMessage = '{} >> Error while unlock AC'.format(
                                time.strftime('%H:%M:%S|%y.%m.%d',time.localtime()))
                            logging.warning(answerMessage)
                        else:#allow blocking
                            conn.send("Unlock AC".encode("utf-8"))
                            answerMessage = '{} >> Anlock AC after fire alarm'.format(
                                time.strftime('%H:%M:%S|%y.%m.%d',time.localtime()))
                            logging.info(answerMessage)
                        finally:#allow blocking
                            gpio_lock.release()
                    else:#data parse
                        conn.send("error: unknown command: ".encode("utf-8"),data)
                        answerMessage = '{} >> Error: unknown command: {}'.format(
                            time.strftime('%H:%M:%S|%y.%m.%d',time.localtime()),data.decode("utf-8"))
                        logging.warning(answerMessage)
                finally:
                    conn.close()
        finally:#fire socket
            sock.close()

class WebThread(threading.Thread):
    
    def __init__(self):
        threading.Thread.__init__(self)
    
    def run(self):
        server_address = ("", 8000)
        httpd = HTTPServer(server_address, CGIHTTPRequestHandler)
        httpd.serve_forever()

class GameThread(threading.Thread):
    def __init__(self,ip):
        threading.Thread.__init__(self)
        self.currentScore = 0
        self.currentTime  = 0
        self.ip = ip
    def run(self):
        ind_serial.open()
        ind_serial.write(b'20\n')
        ind_serial.write(b'1450\n')
        ind_serial.close()
        self.currentTime = time.time()
        while 1:
            self.currentScore = updateRecord(self.currentScore,self.ip)
            ind_serial.open()
            ind_serial.write(b'2' + bytes(str(self.currentScore), encoding='utf-8') + b'\n')
            ind_serial.close()
            if time.time() - self.currentTime > 450:
                break
            time.sleep(1)
        ind_serial.open()
        ind_serial.write(b'4' + bytes(str(self.currentScore), encoding='utf-8') + b'\n')
        ind_serial.write(b'3450\n')
        ind_serial.close()
        self.currentTime = time.time()
        while 1:
            self.currentScore = updateRecord(self.currentScore,self.ip)
            ind_serial.open()
            ind_serial.write(b'4' + bytes(str(self.currentScore), encoding='utf-8') + b'\n')
            ind_serial.close()
            if time.time() - self.currentTime > 450:
                break
            time.sleep(1)
        ind_serial.open()
        #ind_serial.write(b'30\n')
        ind_serial.write(b'4' + bytes(str(self.currentScore), encoding='utf-8') + b'\n')
        ind_serial.close()
        

def createRecord():
    global lastrowid
    data_base = sqlite3.connect('score.db')
    try:
        tmp_cur = data_base.cursor()
        tmp_sql = 'INSERT INTO game (day,time,score) VALUES (\'{0}\',\'{1}\',\'{2}\');'.format(time.strftime('%Y.%m.%d',time.localtime()),time.strftime('%H:%M',time.localtime()),0)
        tmp_cur.execute(tmp_sql)
        lastrowid = tmp_cur.lastrowid
        tmp_cur.close()
        data_base.commit()
    finally:
        data_base.close()
        
def updateRecord(currentScore,ip):
    global lastrowid,ind_serial
    sock_modbus = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    data_base = sqlite3.connect('score.db')
    try:
        sock_modbus.connect(('10.11.13.'+ip, 502))
        str_message = tcp.read_input_registers(slave_id=1,starting_address=102, quantity=1)
        list_response = tcp.send_message(str_message,sock_modbus)
    finally:
        sock_modbus.close()
    if list_response[0] != currentScore:
        tmpScore = list_response[0]
        try:
            tmp_cur = data_base.cursor()
            tmp_sql = ('UPDATE game SET score=\'{0}\' WHERE  id_game=\'{1}\';'.format(tmpScore,lastrowid))
            tmp_cur.execute(tmp_sql)
            tmp_cur.close()
            data_base.commit()
        finally:
            data_base.close()
        return tmpScore
    return currentScore

if __name__ == '__main__':
    os.chdir('/home/pi/AC')
    data_base = sqlite3.connect('score.db')
    try:
        tmp_cur = data_base.cursor()
        tmp_sql = '''\
        SELECT id_game,day,time,score FROM game;
        '''
        tmp_cur.execute(tmp_sql)
    except sqlite3.Error as e:
        data_base.close()
        tmpsolt = time.strftime('%y%m%d%H%M%S',time.localtime())
        os.rename("score.db","score.db.{1}.old".format(e.args[0],tmpsolt ))
        data_base = sqlite3.connect('score.db')
        tmp_cur = data_base.cursor()
        tmp_sql = '''\
            CREATE TABLE game (
            id_game INTEGER PRIMARY KEY AUTOINCREMENT,
            day TEXT,
            time TEXT,
            score INTEGER);
            '''
        tmp_cur.execute(tmp_sql)
        tmp_cur.close()
    finally:
        data_base.close()
    pygame.mixer.init()
    GPIO.setmode(GPIO.BCM)
    GPIO.setup((door_pin,player_pin), GPIO.OUT, initial=GPIO.HIGH)
    logging.basicConfig(filename='ac.log',level=logging.DEBUG)
    WebThread().start()
    SocketThread(serv_addr,Threat_Flags).start()
    app = QApplication(sys.argv)
    ex = PinPad(pinCheck,servPinCheck,openDoor,setDoor)
    sys.exit(app.exec_())
    GPIO.cleanup()
