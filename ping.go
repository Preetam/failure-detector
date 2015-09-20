package main

import (
	"log"
	"net"
)

func main() {
	conn, err := net.Dial("tcp", "127.0.0.1:2020")
	if err != nil {
		log.Fatal(err)
	}
	conn.Write([]byte{0x1, 0x3, 0x0, 0x0, 0x0, 'f', 'o', 'o'})
	var buf [100]byte
	b := buf[:]
	conn.Read(b)
	log.Println(b)
}
