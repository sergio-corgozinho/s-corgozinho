# Program: The Stand Counter
# Author: Sergio Corgozinho
# Last Update: 2022-03-20

# Bibliotecas
import RPi.GPIO as GPIO
import time
import datetime
import csv
import mysql.connector as msql
from rpi_lcd import LCD

import I2C_LCD_driver
import socket
import fcntl
lcdi2c = I2C_LCD_driver.lcd()
lcdi2c.lcd_display_string("Arduino e Cia", 1,1)
lcdi2c.lcd_display_string("LCD I2C e RPi", 2,1)

# GPIO: Sensor Distância, Botões e LEDs
ECHO = 21			# Sensor - Echo
TRIGGER = 20		# Sensor - Trigger
b1 = 16				# Botão 1 - Inicia Contagem
b2 = 12				# Botão 2 - Para contagem
b3 = 7				# Botão 3 - Sincronizar com BD
led_on = 8			# LED indicador de leitura ligada
led_count = 25			# LED indicador de contagem

# GPIO: Driver Motores
mot_A_PWM = 26;		
mot_A_IN2 = 19;		
mot_A_IN1 = 13;		
mot_standby = 6;	
mot_B_IN1 = 5;		
mot_B_IN2 = 11;		
mot_B_PWM = 9;		

# Variáveis de Controle
lcd = LCD()			# LCD
play = 0			# Inicio e fim da leitura do sensor
count = 0			# Contagem das plantas
dist_max = 40		# Distância máxima para contagem
dist_min = 50		# Distância minima para contagem
dist_set = 0		# Auxiliar de nova contagem

# Definindo Dados do Usuário e Fazenda
farm_id = 2		
user_id = 3

# GPIO Setup
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(1)
GPIO.setup(TRIGGER,GPIO.OUT)
GPIO.output(TRIGGER,0)
GPIO.setup(ECHO,GPIO.IN)
GPIO.setup(b1,GPIO.IN)
GPIO.setup(b2,GPIO.IN)
GPIO.setup(b3,GPIO.IN)
GPIO.setup(led_on,GPIO.OUT)
GPIO.output(led_on,0)
GPIO.setup(led_count,GPIO.OUT)
GPIO.output(led_count,0)
GPIO.setup(mot_A_PWM,GPIO.OUT)
GPIO.setup(mot_A_IN2,GPIO.OUT)
GPIO.setup(mot_A_IN1,GPIO.OUT)
GPIO.setup(mot_standby,GPIO.OUT)
GPIO.setup(mot_B_IN1,GPIO.OUT)
GPIO.setup(mot_B_IN2,GPIO.OUT)
GPIO.setup(mot_B_PWM,GPIO.OUT)

# Setup Arquivo 
distance_csv = "/home/pi/CORGOZINHO/stand_counter/csv_data/distance.csv"
count_csv = "/home/pi/CORGOZINHO/stand_counter/csv_data/count.csv"
f_distance = open(distance_csv, 'a')
f_count = open(count_csv, 'a')

# Cabeçalho Inicial
lcd.text("*STAND COUNTER*", 1)
lcd.text("by CorgozinhoDev", 2)
print("\n****Bem-vindo ao Stand Counter!***")
print("\n **BY CORGOZINHO MEGA DEV**\n\n")
time.sleep(2)
 
# Código Principal
try:	
	#print("B1: " + str(GPIO.input(b1)))
	#print("B2: " + str(GPIO.input(b2)))
	#print("B3: " + str(GPIO.input(b3)))
	
	print("\nPressione o botão 1 para iniciar a contagem...\n")
	lcd.text("Pressione B1", 1)
	lcd.text("para iniciar", 2)

	while play == 0:
		if GPIO.input(b1) == 1:
			play = 1
	
	first_timestamp = datetime.datetime.now()
	while play==1:
		GPIO.output(led_on,1)
		
		# Medir distância
		GPIO.output(TRIGGER,True)
		time.sleep(0.00001)
		GPIO.output(TRIGGER,False)
		start = time.time()
		stop = time.time()
		while GPIO.input(ECHO)==0:
			start = time.time()
		while GPIO.input(ECHO)==1:
			stop = time.time()
		elapsed = stop - start
		distance = (elapsed * 34300)/2		#### D = (T x V)/2
		
		ct = datetime.datetime.now()
		print("Distance: " + str(distance) + ", Current time: " + str(ct))
		
		# Contagem do estande
		if distance > dist_max: dist_set = 0
		if (distance <= dist_min and dist_set == 0):
			count = count + 1
			dist_set = 1
			GPIO.output(led_count,1)
		
		lcdi2c.lcd_clear()
		lcdi2c.lcd_display_string("Distancia: %s" %str(distance), 1,1)
		lcdi2c.lcd_display_string("COUNT: %s" %str(count), 2,1)

		GPIO.output(led_count,0)
		
		# Armazena distâncias no csv
		writer = csv.writer(f_distance)
		writer.writerow([ int(first_timestamp.strftime("%Y%m%d%H%M%S")), str(round(distance, 3)), str(ct) ])
		
		if GPIO.input(b2) == 1:
			play = 0

		time.sleep(0.05)                    

except KeyboardInterrupt:
	print ("quit")

GPIO.output(led_on,0)
last_timestamp = datetime.datetime.now()
print("\nCount: " + str(count) + "\n")
lcdi2c.lcd_clear()
lcdi2c.lcd_display_string("TOTAL COUNT: %s" %str(count), 1,1)

# Armazena contagem no csv
writer = csv.writer(f_count)
writer.writerow([ int(first_timestamp.strftime("%Y%m%d%H%M%S")), str(farm_id), str(user_id), str(count), str(first_timestamp), str(last_timestamp) ])

f_distance.close()
f_count.close()

lcd.text("Salvando BD...", 2)
print("Escrevendo no Banco de Dados. \nAguarde por favor...\n")
db = msql.connect(host="3.82.250.107", user="stand_counter", passwd="123456", db="stand_counter")
cur = db.cursor()

with open(count_csv) as csvFile:
	csv_data = csv.DictReader(csvFile, delimiter=',')
	for row in csv_data:
		cur.execute("INSERT INTO counter (id,farm_id,user_id,count,first_timestamp,last_timestamp) VALUES (%s, %s, %s, %s, %s, %s);", [int(row["id"]), int(row["farm_id"]), int(row["user_id"]), int(row["count"]), str(row["first_timestamp"]), str(row["last_timestamp"])])
		db.commit()

print("Deseja salvar no banco de dados as distâncias medidas?")
print("B1 = sim")
print("B2 = não")
lcd.clear()
lcd.text("Salvar dists?", 1)
lcd.text("b1=sim b2=nao", 2)

while True:
	if GPIO.input(b1) == 1:
		print("Salvando distâncias")
		lcd.clear()
		lcd.text("Salvando no BD", 1)
		lcd.text("Aguarde...", 2)
		with open(distance_csv) as csvFile:
			csv_data = csv.DictReader(csvFile, delimiter=',')
		
			for row in csv_data:
				cur.execute("INSERT INTO distance (count_id, distance, timestamp) VALUES (%s, %s, %s);", [int(row["count_id"]), float(row["distance"]), str(row["timestamp"])])
				db.commit()
		print("\nDados armazenados com sucesso!")
		lcd.clear()
		lcd.text("Dados salvos!", 1)
		time.sleep(3)
		break
	
	if GPIO.input(b2) == 1:
		print("\nApenas a contagem foi salva.")
		lcd.clear()
		lcd.text("Apenas contagem", 1)
		lcd.text("foi salva", 2)
		time.sleep(3)
		break
	
cur.close()

# limpar os arquivos CSV após escrita no BD
f = open(count_csv, "w")
f.truncate()
f.close()

f = open(distance_csv, "w")
f.truncate()
f.close()

with open (count_csv,'a') as csvFile:
	writer = csv.writer(csvFile, delimiter=",")
	writer.writerow(['id', 'farm_id', 'user_id', 'count', 'first_timestamp', 'last_timestamp'])

with open (distance_csv,'a') as csvFile:
	writer = csv.writer(csvFile, delimiter=",")
	writer.writerow(['count_id', 'distance', 'timestamp'])

print("Finalizando aplicação...")
print("\nObrigado por utilizar The Stand Counter!\n")
lcd.clear()
lcd.text(" **Thank you!**", 1)
lcd.text("    BYE BYE", 2)
time.sleep(5)
GPIO.cleanup()
lcd.clear()
