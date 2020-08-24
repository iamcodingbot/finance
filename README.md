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

i am working on mudminer contract. this is where user will deposit their LP tokens for MUD rewards. here i will also issue them VOTING tokens, which they will use to update CR + others. 

these actions need to be called in this order, otherwise there will be failure - 
starttxn - creates an entry for incoming transfer action. 
receiveXXX - these methods are called on notification of xfers. updates the entry created in starttxn. 
