#ifndef ORDER_H
#define ORDER_H

// Define the orders that can be sent and received
enum Order {
  HELLO = 0,
  ALREADY_CONNECTED = 1,
  RECEIVED = 2,
  FORCES = 3,
  POSITION = 4,
  ERROR = 5,
  LOG = 6,
  VERSION = 7
};

typedef enum Order Order;

#endif
