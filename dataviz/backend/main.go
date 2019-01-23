package main

import (
	"log"
	"strings"

	"github.com/tarm/serial"
)

func main() {
	c := &serial.Config{Name: "/dev/tty.usbmodem14201", Baud: 115200}
	s, err := serial.OpenPort(c)
	if err != nil {
		log.Fatal(err)
	}

	// read data to buffer
	rowBuffer := make([]byte, 256)
	for {
		buf := make([]byte, 256)
		n, err := s.Read(buf)
		if err != nil {
			log.Fatal(err)
		}
		rows := strings.Split(string(buf[:n]), "\r\n")

		for _, b := range []byte(rows[len(rows) - 1]) {
			rowBuffer = append(rowBuffer, b)
		}
		
		rows = rows[:len(rows) - 1]
		for _, row := range rows {
			valueCount := len(strings.Split(row, ";"))
			if valueCount <= 12 {
				if valueCount == 12 {
					log.Printf("%s", row)
				} else {
					stringBuffer = string
				}
			}
			log.Printf("%d", valueCount)
		}
	}

	//log.Printf("%q", strings.Split(string(buf[:n]), "\r\n"))
}
