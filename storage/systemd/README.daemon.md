# Systemd service file

## Install

Edit path in the line 12 in the "lorawan-identity-storage.service" file:

```
ExecStart=/home/andrei/src/ws-lora/build/lora-ws -d --pidfile /var/run/lorawan-identity-storage.pid
```

Copy file and start service:

```
sudo ./install.sh
sudo systemctl start lorawan-identity-storage.service
systemctl daemon-reload
```

## Reference

[Writing systemd service files](https://patrakov.blogspot.com/2011/01/writing-systemd-service-files.html)
