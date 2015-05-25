# DmQ-Adapter

In the early 1990s I was a lead developer of DECmesssageQ (aka BEAmessageQ
or DEC PAMS), the 1st multi-vendor, multi-platform message queuing
communications product. At the time vendors like IBM had message queuing
products, but only for their own platforms. DmQ was the first to allow
customers to create a message bus (using publish-subscribe architecture
with notifications and receipts) across any vendor's system.

This archive contains the "DmQ Adapter" (aka "PAMS TCPIP Adapter"), a
stand-alone RPC add-on to DECmessageQ that I wrote entirely (including the
documentation), allowing odd and unsupported platforms to communicate on
the message bus using TCP/IP Sockets. It was a tool given away to help
customers and not part of the DmQ product.

I was quite proud of this work, as I wrote it in portable C (ANSI and K&R)
"endian independent" code compatible with 32bit, 64bit, and RISC
processsors. There were no compile variants in the code; just a platform
specific include file that defined how the code would compile and run.

Note that this adapter requires a running DECmessageQ/BEAmessageQ/DECPAMS
system with at least one connected VAX/VMS system to host the server
component.

- Keith
