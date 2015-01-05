#ifndef HKJ_EXCEPTION_H
#define HKJ_EXCEPTION_H

class HKJException {};

class HKJError {};

class SocketFailed : public HKJError {};
class BindFailed : public HKJError {};
class ConnectFailed : public HKJError {};
class ForkFailed : public HKJError {};
class ReadFailed : public HKJError {};
class WriteFailed : public HKJError {};

#endif