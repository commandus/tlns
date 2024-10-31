# Send payload to device CLI

This directory contains source files of the parser used in TcpUdpV4Bridge example.

Commands:

- ping- ping service
- send- send payload and/or FOpts immediately or at specified time

## ping

Ping service
```
ping
```
The service does not send anything in response.

## send

```
--send--+-------------+----+--+---+--+--+--
        |    1..N     |    |  |   |  |  |
        +--<address>--+    |  |   |  |  |
        |                  |  |   |  |  |
        +--fport <number>--+  |   |  |  |
        |                     |   |  |  |
        +--proto--<number>----+   |  |  |
        |                         |  |  |
        +--payload--<hex-string>--+  |  |
        |                            |  |
        +--fopts--<hex-string>-------+  |
        |                               |
        +--at--<date-time>--------------+
```

Specify one or more addresses.

```
send {<address>} [fport <uint>] [proto <uint>] [payload <hex-string>] [fopts <hex-string>] [at <date-time>] 
```

- fport FPort number 0..255. By default 1.
- proto Protocol number. By default 0. 
- at date-time is date & time stamp in format:
```
YYYY-MM-DDThh:mm:ssTZ
```
where
- YYYY year
- MM month 01..12
- DD day 01..31
- T time separator
- hh hours 0..23
- mm minutes 0..59
- ss seconds 0..59
- TZ time zone shift in hours

For instance, 2024-10-31T00:00:00+09 is October 31 2024 midnight in time zone +9.
