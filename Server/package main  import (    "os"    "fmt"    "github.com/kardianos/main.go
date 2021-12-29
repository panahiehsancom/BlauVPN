package main

import (
   "os"
   "fmt"
   "github.com/kardianos/service"
   "time"
)

const serviceName = "Medium service"
const serviceDescription = "Simple service, just for fun"

type program struct{}

func WriteToFile(filename string, data string) error {
    file, err := os.OpenFile(filename, os.O_APPEND|os.O_WRONLY|os.O_CREATE, 0600)
    if err != nil {
        file.Close()
        return err
    }

    _, err = file.WriteString(data)
    if err != nil {
        file.Close()
        return err
    }
  return  file.Close()
}

func (p program) Start(s service.Service) error {
   fmt.Println(s.String() + " started")
  WriteToFile("C:\\goservice.txt","Service Started\n")
   go p.run()
   return nil
}

func (p program) Stop(s service.Service) error {
  WriteToFile("C:\\goservice.txt","Service Stopped\n")
   fmt.Println(s.String() + " stopped")
   return nil
}

func (p program) run() {
   for {
       WriteToFile("C:\\goservice.txt","Service Running\n")
      fmt.Println("Service is running")
      time.Sleep(1 * time.Second)
   }
}

func main() {


    WriteToFile("C:\\goservice.txt","Calling Main Function\n")


   serviceConfig := &service.Config{
      Name:        serviceName,
      DisplayName: serviceName,
      Description: serviceDescription,
   }
   prg := &program{}
   s, err := service.New(prg, serviceConfig)
   if err != nil {
      fmt.Println("Cannot create the service: " + err.Error())
   }
   err = s.Run()
   if err != nil {
      fmt.Println("Cannot start the service: " + err.Error())
   }
}
