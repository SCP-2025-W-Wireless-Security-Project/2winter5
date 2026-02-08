# dash.py
from flask import Flask, render_template
import subprocess
import json

app = Flask(__name__)

@app.route('/')
def dashboard():
    # 클라이언트 정보 가져오기
    clients = subprocess.getoutput("arp -a")
    dhcp = subprocess.getoutput("cat /var/lib/misc/dnsmasq.leases")
    
    return render_template('dashboard.html', 
                         clients=clients, 
                         dhcp=dhcp)

if __name__ == '__main__':
    app.run(host='192.168.100.1', port=5000)
