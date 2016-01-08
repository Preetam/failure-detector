package main

import (
	"flag"
	"log"
	"net"
	"time"
)

func main() {
	address := flag.String("address", "127.0.0.1:2020", "address of failure-detector")
	flag.Parse()
	conn, err := net.Dial("tcp", *address)
	if err != nil {
		log.Fatal(err)
	}
	//var buf [100]byte

	for _ = range time.Tick(time.Second) {
		_, err := conn.Write([]byte{
			// Length
			14, 0x0, 0x0, 0x0,
			// Type (MSG_PING)
			0x1,
			// ID
			0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
			// No body
		})
		if err != nil {
			return
		}
	}
}
