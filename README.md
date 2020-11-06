# Brick finance

Brick finance is two coin ecosystem inspired by Frax.finance - https://frax.finance/whitepaper/

There are some differences in implementation -
1) This implementation pegs 1 BRICK with 1 USDT.
2) MUD, BRICK and stable coins prices are calculated from liquidity pools.

Brick is an eosio implementation.

## Tokens 

**BRICK** - Brick is the low volatility coin in the system. Total initial supply of BRICK is 1 Billion. [TODO - can this be more than 1 b?] 

**MUD** - Mud is the volatile coin in the system. Total mineable supply is 30000 MUD. MUD sent to smart contract for getting BRICK tokens are removed from circulation. When collateral is redeemed, MUD supply is increased. 

## Current Ratio or Collateral Ratio (CR)

This signifies the ratio at which BRICK can be minted and collateral can be redeemed. CR can range from 0.5[TODO check the lower value] to 100.0  

Varying values of CR control how brick, mud and collateral interact.

**When CR is 100% :**
- Collateral redeem smart contract  - 1 Brick can redeem 1 USDT worth of collateral from a smart contract.
- Brick mint smart contrat - 1 Brick can be minted by sending 1 USDT worth of collateral to a smart contract.

**When CR < 100% :**
Say when the CR is 90%
- Collateral redeem smart contract - 1 BRICK can be redeemed to get .90 USDT worth of collateral and .10 USDT worth of MUD. New MUD is generated to capture this price.
- Brick mint smart contract - 1 Brick can be minted by sending .90 USDT worth of collateral and .10 USDT worth of MUD. This MUD is taken out of circulation.

Collateral redeem smart contract charges 0.05% fees. This fees can be used by MUD/BRICK holders to use for anything.

## How does the CR value change?

The CR increase/decrease based on decrease/increase of BRICK supply. CR value is inversely related to BRICK supply.

## How can MUD be mined?

Initially MUD can be mined by providing liquidity at a swap exchange which issues LP tokens. Currently, EIDOS and Alcor.exchange are coded.
Subsequently, providing MUD/BRICK for lending on PIZZA others may also mine MUD. MUD mining will go for a limited period of time, till the time vault is developed OR cap of MUD is reached.

## Which stable coins are supported as collateral?

Initially USDT is supported in code. There is plan to support other stable coins like VIGOR. When supported, 1 USDT worth of VIGOR can be used to mint 1 BRICK when the CR is 100%. 
Other stable coins can be included via voting.

## Is there a cap on total number of tokens that can be deposited for a single collateral?

Each collateral has upper limit for deposits. This limit can be increased via voting.

## Can the deposited collateral generate yield?

Deposit collateral can generate yield as goverened by MUD/BRICK holders.

## How is the price of collateral token calculated?

Price of stable coin is calculated using stable.sx smart contract.

## How is the price of MUD/BRICK calculated?

Price of MUD and BRICK are calculated using volume weighted price from multiple swaps smart exchanges. 

# CR calculation

Let CR be represented by c.
Let Supply at a CR represented by s.

Supply till which CR remains 100 be represted by M.
Let E be Euler constant. It can be any integer. 

Also, c = 100 when s <= M

T is target supply such that c = 0
i.e. c = 0 when s >= T

Relation between s, c can be calculated via combination of following equations -
c = 0                   WHEN s > T
c = 100                 WHEN s =< M
k * c pow E + M = s     WHEN s <= T 

**THIS IS AN EXPERIMENTAL IMPLEMENTATION**
