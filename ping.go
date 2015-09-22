package main

import (
	"log"
	"net"
	"time"
)

func main() {
	conn, err := net.Dial("tcp", "127.0.0.1:2020")
	if err != nil {
		log.Fatal(err)
	}
	var buf [100]byte

	for _ = range time.Tick(time.Second) {
		conn.Write([]byte{0x1, 0x3, 0x0, 0x0, 0x0, 'f', 'o', 'o'})
		b := buf[:]
		conn.Read(b)
		log.Println(b)
	}
}
