# finance



Foundary Contract -   
To get new brick user calls a txn which consists of -

1)starttxn action.
2)transfer USDT/VIGOR action. receiveusdt/receivevigor methods are called on these trasnfers,
3)transfer MUD action. receivemud method is called on the transfer.
4_settle

all these txn work on a single row in table bricktxn. this table maintains changes after every action.

To redeem brick, user calls transfer brick action. receivebrick action is called on transfer.

brickissuer, mudissuer - handles issue, retire, xfer of BRICK and MUD.

foundarydac- this has updateCR. but i will modify this be updated based on user voting.


LP tokens from Alcor, Eidos are supported for MUD mining. 
