# Brick finance

Brick finance is two coin system inspired by Frax.finance - https://frax.finance/whitepaper/

There are some differences in implementation -
1) This implementation pegs the 1 BRICK with 1 USDT.
2) The prices are driven from liquidity pools.
3) community unlocks the voting on CR after certain number of muds are mined. 

## Tokens 

**BRICK** - Brick is the low volatility coin in the system. Total initial supply of BRICK is 1 Billion. [TODO - can this be more than 1 b?] 

**MUD** - Mud is the volatile coin in the system. Total mineable supply is 30000 MUD. MUD sent to smart contract for getting BRICK tokens are removed from circulation. When collateral is redeemed, MUD supply is increased. 

## Current Ratio or Collateral Ratio (CR)

This signifies the ratio at which BRICK can be minted and collateral can be redeemed. CR can range from 0.5[TODO check the lower value] to 100.0  

Varying values of CR controls how the system works.

**When CR is 100% :**
- Collateral redeem smart contract  - 1 Brick can redeem 1 USDT worth of collateral from a smart contract.
- Brick mint smart contrat - 1 Brick can be minted by sending 1 USDT worth of collateral to a smart contract.

**When CR < 100% :**
Say when the CR is 90%
- Collateral redeem smart contract - 1 USDT worth of collateral can be redeemed by sending .90 BRICK and .10 USDT worth of MUD. This MUD is taken out of circulation.
- Brick mint smart contract - 1 Brick can be minted by sending .90 USDT worth of collateral and .10 USDT worth of MUD. New MUD is generated to capture this price.

## How does the CR value change?

After a certain number of MUD is minted, CR voting module is unlocked for BRICK/MUD holders to vote for CR direction (up, down, same). Every 8 hours, CR direction can move by .5% 
In the subsequent version, CR change by bidding can be implemented. 

## How can MUD be mined?

By providing liquidity at a swap exchange which issues LP tokens, MUD can be mined. Currently, EIDOS and Alcor.exchange are considered.

## Which stable coins are supported as collateral?

Initially USDT is supported in code. There is plan to support other stable coins like VIGOR. 1 USDT worth of VIGOR can be used to mint, 1 BRICK when the CR is 100%. 
Other stable coins can be included via voting.

## Is there a cap on total number of coins that can be deposited as a collateral?

Each collateral has upper limit for deposits. This limit can be increased via voting.

## Can the deposited collateral generate yield?

Deposit collateral can generate yield as goverened by MUD/BRICK holders.

**THIS IS AN EXPERIMENTAL IMPLEMENTATION**
