#!/bin/sh
# Create PID file
touch /var/run/lorawan-identity-storage.pid
# Copy systemd service file
cp lorawan-identity-storage.service /etc/systemd/system/

systemctl start lorawan-identity-storage.service
systemctl daemon-reload

exit 0
