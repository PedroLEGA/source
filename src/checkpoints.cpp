// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2017 The Signatum Project www.signatum.io
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "txdb.h"
#include "main.h"
#include "uint256.h"


static const int nCheckpointSpan = 500;

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        (  0,     uint256("0x00000c325a3586e495e56bccedf9a8bf6dc21d91df7a8ae1151627c2c3351c99") ) // here we go
        (  500,     uint256("0xc842ceeb227fafaeb90c9b3c820da8f2c1d7debd77ab9f879bf975add00c48ab") ) // init first checkpoint
        (  750,     uint256("0xbb5b5f8f4052dde501d2aad794c49347222a7f900b1097607e0492a4d11bd287") )
        (  1250,     uint256("0x6dc761812a65430115dd49a6da3ffc1e4f66bdb089a30fe43a12c7bd60f5ed0d") )
        (  1810,     uint256("0xb524c9c69bad926472b04138c2d3c934045d4d0be9199a042ecf794a82e1799f") )
        (  3423,     uint256("0x6552c3bca51a03e822486f45b4d53eb58b33609027bcad5f11b9ca18f831d9fb") )
        (  4562,     uint256("0x2c1cddfa9dea9b65f2d04d21bcac21cbf39f139579ca45fd43a490fa171fe94b") )
        (  6267,     uint256("0xcb4d96a1811afe263e179fd7d2d41e4f61847dc2c0895d3f36525f3c3a926643") )
        (  9224,     uint256("0xea596b516a17fb4b46954d391e825d3976a14011b8e5020f3615d5cf13a2f12e") )
        (  14129,     uint256("0xb8c42b0ff384a45edc43da500dd2f3d5bcc73021310457832d6ce25423f7c79a") )
        (  19990,     uint256("0x6a5d35d862dd5ac0aed23ebb57b9d9bdd8a9afa1e5079f529a77ebd1bba7ad46") )
        (  26445,     uint256("0x8d55089d1aaa6e1d2cb7edd7a76db43876c6c07c164f47ec9df3a31d88017741") )
        (  29909,     uint256("0x07818e1ed7fc0479e5bd811583a11edce8f455a63aaad3e2956176ec8a052617") )
        (  30005,     uint256("0x86e3dd64f4c82325390e8bb584f721515ee186e4f67195c26b6567fc0a8078c2") ) // halving
        (  32510,     uint256("0x62262d8f7fec52db4dc76ad9ace2cc9ec8989777e6d23e1417ffb6a4f2479733") )
        (  38250,     uint256("0x742d6e65dda953576b4826dd07024cf18e1a7532547341bdd3c151308193b098") )
        (  55353,     uint256("0x646e185bf3afbd01eba9df0b6eb96165ac2797c3991a3ff29e24c158797da2a1") )
        (  65057,     uint256("0x8c7febd2bb382871835b690a4f9509bc32f77829ae4442fdeccf8592a52fbed5") ) // +second halving
        (  77200,     uint256("0x0e03d2e2028cf325fbdb87d33ec57fef018a8c06a2e5648f5bd9f7a2d678036c") )
        (  85945,     uint256("0x34278dac6c9d08ee4d2cc4091dbb6af7eb0e1b3b0bfcd4791d0dd25dceaad9b8") )
        (  88805,     uint256("0x9467b60a1e7783f5056bfef714a428fbb398abc189fc2a871068da0ae042992c") )
        (  97890,     uint256("0x5ac73b4fbe798ea7660401099101ed62caaab94c234944f3074fdef6174c7e4d") ) // nearing PoS Stage
        (  98500,     uint256("0x22321632765b6483e49e0e470b3f45a95fe3b2ef46b9eba900210a49c4407497") ) // PoS stage block
        (  99127,     uint256("0x9c62183731de95bcd5114973b987930b4efd225ac78be6d4788e55f566ac8494") ) // beyond PoS trigger
        (  100590,    uint256("0x4064f1aee53b5306532b8cdb6e058ab79f899fa85ce15f7c27c7a2b62e6691b0") ) // full PoS checkpoint -1
        (  115600,    uint256("0xe1b005f6ecff0a4a7b9db14aba327ebfd820da43be1672dc024c9159030d7a0e") )
        ;

    // TestNet has no checkpoints
    static MapCheckpoints mapCheckpointsTestnet;

    bool CheckHardened(int nHeight, const uint256& hash)
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : mapCheckpoints);

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    int GetTotalBlocksEstimate()
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : mapCheckpoints);

        if (checkpoints.empty())
            return 0;
        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : mapCheckpoints);

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }

    // Automatically select a suitable sync-checkpoint 
    const CBlockIndex* AutoSelectSyncCheckpoint()
    {
        const CBlockIndex *pindex = pindexBest;
        // Search backward for a block within max span and maturity window
        while (pindex->pprev && pindex->nHeight + nCheckpointSpan > pindexBest->nHeight)
            pindex = pindex->pprev;
        return pindex;
    }

    // Check against synchronized checkpoint
    bool CheckSync(int nHeight)
    {
        const CBlockIndex* pindexSync = AutoSelectSyncCheckpoint();

        if (nHeight <= pindexSync->nHeight)
            return false;
        return true;
    }
}
