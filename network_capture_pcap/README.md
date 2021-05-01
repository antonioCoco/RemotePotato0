## Notes

### Scenario:

- victim server: server1 (192.168.83.131)
- domain controller: dc1 (192.168.83.135)
- attacker: kali (192.168.83.130)

Other notes:

- the core of this exploit generate traffic on the victim server (victim_server.pcapng);
- all the irrelevant traffic has been filtered out from the full capture using the following filter:
  - `(dcerpc or http) and not rpc_netlogon and not tcp.port == 5985 and (ip.addr == 192.168.83.130 or ip.addr == 127.0.0.1)`
- i have added the capture also of the domain controller (domain_controller.pcapng). Consider that the LDAP traffic generated on the targeted domain controller is specific to the attack scenario we have used. Different attack scenario could generate different traffic on the targeted machine, e.g. relay to SMB.
