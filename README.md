# finance

Inspired by Frax.finance's whitepaper.

It is two coin system. Stable coin BRICK and investment coin MUD.

System is controlled by collateral ratio(CR), initial value set to 100%. 

At CR = 100% -
  to get 1 BRICK user has to deposit 1 USDT. 
  User can deposit 1 BRICK and get 1 USDT back.

When CR becomes less than 100%, say 99%, -
  to get 1 BRICK user needs to deposit .99 USDT and .01 USDT worth of MUD. This MUD is burnt.
  on selling 1 BRICK user get .99 USDT and .01 worth of MUD at this time. This is newly minted MUD.
  
LP in EOS/BRICK and EOS/MUD pair will be rewarded with MUD tokens.

Goal of the project is to make a fractional reserve system like Federal reserve. Dollar was initially backed by Gold and then the fractional reserve kicked in.

In the initial release CR will be governed by MUD/BRICK holders.
