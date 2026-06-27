# 🔍 Analyse des Écarts ED2K/Kademlia - Envy vs eMule

**Date:** 16 janvier 2026
**Version:** 1.0
**Objective:** Detailed comparison of Envy's ED2K/Kademlia implementation with eMule (reference)

---

## 📋 Overview

**Référence:** eMule 0.60+ (dernière version stable)
**Envy:** Implémentation partielle ED2K/Kademlia
**Objective:** Functional parity with eMule for complete interoperability

---

## 🔴 Messages Kademlia Manquants

### Messages UDP Kademlia (ED2K_PROTOCOL_KAD)

#### Implémentés dans Envy
- ✅ `KADEMLIA2_BOOTSTRAP_REQ` - `Envy/Kademlia.cpp:416-462`
- ✅ `KADEMLIA2_BOOTSTRAP_RES` - `Envy/Kademlia.cpp:464-528`
- ✅ `KADEMLIA2_PING` - `Envy/Kademlia.cpp:530-544`
- ✅ `KADEMLIA2_PONG` - `Envy/Kademlia.cpp:546-561`
- ✅ `KADEMLIA2_REQ` (FIND_NODE) - `Envy/Kademlia.cpp:563-629`
- ✅ `KADEMLIA2_RES` (FIND_NODE) - `Envy/Kademlia.cpp:631-682`

#### Manquants dans Envy

**1. KADEMLIA2_HELLO_REQ / KADEMLIA2_HELLO_RES**
- **Usage:** Établissement de contact initial avec ID de nœud
- **Format eMule:**
  ```
  HELLO_REQ: <TargetID(16)><TCPPort(2)><Version(1)><UDPPort(2)>
  HELLO_RES: <TargetID(16)><TCPPort(2)><Version(1)><UDPPort(2)><TagCount(1)><Tags...>
  ```
- **Impact:** Contacts ne peuvent pas s'identifier correctement
- **File to modify:** `Envy/Kademlia.cpp` - Add handlers
- **Complexity:** Low (S)

**2. KADEMLIA2_REQ (FIND_VALUE)**
- **Usage:** Recherche de valeurs (fichiers, mots-clés) dans le DHT
- **Format eMule:**
  ```
  <Type(1)><TargetID(16)><ReceiverID(16)>
  Type: KADEMLIA_FIND_VALUE (0x02)
  ```
- **Impact:** Recherche de fichiers dans Kademlia non fonctionnelle
- **Fichier à modifier:** `Envy/Kademlia.cpp:563-629` - Étendre `OnFindNodeRequest()`
- **Complexity:** Medium (M)

**3. KADEMLIA2_REQ (STORE)**
- **Usage:** Publication de fichiers/mots-clés dans le DHT
- **Format eMule:**
  ```
  <Type(1)><TargetID(16)><ReceiverID(16)><SourceID(16)><Load(1)><Data...>
  Type: KADEMLIA_STORE (0x04)
  ```
- **Impact:** Partage de fichiers via Kademlia non fonctionnel
- **Fichier à modifier:** `Envy/Kademlia.cpp` - Ajouter handler
- **Complexity:** Medium (M)

**4. KADEMLIA2_PUBLISH_REQ / KADEMLIA2_PUBLISH_RES**
- **Usage:** Publication de sources de fichiers
- **Format eMule:**
  ```
  PUBLISH_REQ: <TargetID(16)><Load(1)><FileCount(2)><Files...>
  PUBLISH_RES: <TargetID(16)><Load(1)>
  ```
- **Impact:** Sources de fichiers non publiées dans DHT
- **File to modify:** `Envy/Kademlia.cpp` - Add handlers
- **Complexity:** Medium-High (M-L)

**5. KADEMLIA2_FIREWALLED_REQ / KADEMLIA2_FIREWALLED_RES**
- **Usage:** Détection et communication avec nœuds firewalled
- **Format eMule:**
  ```
  FIREWALLED_REQ: <TargetID(16)><TCPPort(2)>
  FIREWALLED_RES: <TargetID(16)><TCPPort(2)><IP(4)><UDPPort(2)>
  ```
- **Impact:** Nœuds firewalled non contactables
- **File to modify:** `Envy/Kademlia.cpp` - Add handlers
- **Complexity:** Medium (M)

**6. KADEMLIA2_FINDBUDDY_REQ / KADEMLIA2_FINDBUDDY_RES**
- **Usage:** Recherche de "buddies" (contacts de confiance)
- **Format eMule:**
  ```
  FINDBUDDY_REQ: <TargetID(16)><BuddyID(16)>
  FINDBUDDY_RES: <TargetID(16)><BuddyID(16)><IP(4)><Port(2)>
  ```
- **Impact:** Système de buddies non fonctionnel
- **File to modify:** `Envy/Kademlia.cpp` - Add handlers
- **Complexity:** Low (S)

**7. KADEMLIA2_CALLBACK_REQ / KADEMLIA2_CALLBACK_RES**
- **Usage:** Callback pour nœuds firewalled
- **Format eMule:**
  ```
  CALLBACK_REQ: <TargetID(16)><ConnectID(16)><TCPPort(2)>
  CALLBACK_RES: <TargetID(16)><ConnectID(16)><IP(4)><Port(2)>
  ```
- **Impact:** Callbacks Kademlia non fonctionnels
- **File to modify:** `Envy/Kademlia.cpp` - Add handlers
- **Complexity:** Medium (M)

---

## 🔴 Messages ED2K Manquants

### Messages TCP ED2K (ED2K_PROTOCOL_EDONKEY / ED2K_PROTOCOL_EMULE)

#### Implémentés dans Envy
- ✅ `ED2K_C2C_HELLO` - `Envy/EDClient.cpp:1021-1023`
- ✅ `ED2K_C2C_HELLOANSWER` - `Envy/EDClient.cpp:1024-1025`
- ✅ `ED2K_C2C_FILEREQUEST` - `Envy/EDClient.cpp:1028-1029`
- ✅ `ED2K_C2C_FILEREQANSWER` - `Envy/EDClient.cpp:1044-1046`
- ✅ `ED2K_C2C_FILENOTFOUND` - `Envy/EDClient.cpp:1047-1049`
- ✅ `ED2K_C2C_FILESTATUS` - `Envy/EDClient.cpp:1050-1052`
- ✅ `ED2K_C2C_QUEUEREQUEST` - `Envy/EDClient.cpp:1034-1035`
- ✅ `ED2K_C2C_QUEUERANK` - `Envy/EDClient.cpp:1056-1058`
- ✅ `ED2K_C2C_STARTUPLOAD` - `Envy/EDClient.cpp:1059-1061`
- ✅ `ED2K_C2C_FINISHUPLOAD` - `Envy/EDClient.cpp:1062-1064`
- ✅ `ED2K_C2C_SENDINGPART` - `Envy/EDClient.cpp:1065-...`

#### Manquants dans Envy

**1. ED2K_C2C_SECIDENTSTATE / ED2K_C2C_SIGNATURE**
- **Usage:** Authentification SecureID eMule
- **Format eMule:**
  ```
  SECIDENTSTATE: <Challenge(6)>
  SIGNATURE: <Response(16)> (MD5 hash)
  ```
- **État actuel:** Code présent mais incomplet - `Envy/EDClient.cpp:64-159`
- **Problèmes:**
  - Utilise `rand()` au lieu de RNG cryptographique
  - Hash simple au lieu de MD5
- **Fichier à modifier:** `Envy/EDClient.cpp:64-159` - Compléter implémentation
- **Complexity:** Low (S) - Corriger RNG et hash

**2. ED2K_C2C_EMULEINFO / ED2K_C2C_EMULEINFOANSWER**
- **Usage:** Échange d'informations eMule (version, capacités)
- **Format eMule:**
  ```
  EMULEINFO: <Version(4)><Capabilities(4)><Tags...>
  EMULEINFOANSWER: <Version(4)><Capabilities(4)><Tags...>
  ```
- **Impact:** Compatibilité limitée avec clients eMule modernes
- **File to modify:** `Envy/EDClient.cpp` - Add handlers
- **Complexity:** Low (S)

**3. ED2K_C2C_COMPRESSEDPART / ED2K_C2C_COMPRESSEDPART_I64**
- **Usage:** Envoi de parties compressées (zlib)
- **Format eMule:**
  ```
  COMPRESSEDPART: <Hash(16)><Offset(4)><CompressedSize(4)><UncompressedSize(4)><Data...>
  COMPRESSEDPART_I64: <Hash(16)><Offset(8)><CompressedSize(4)><UncompressedSize(4)><Data...>
  ```
- **Impact:** Performance réduite (pas de compression)
- **File to modify:** `Envy/UploadTransferED2K.cpp` - Add compression
- **Complexity:** Medium (M)

**4. ED2K_C2C_QUEUERANKING**
- **Usage:** Classement de file d'attente amélioré (eMule)
- **Format eMule:**
  ```
  QUEUERANKING: <Rank(4)><WaitingCount(4)><Tags...>
  ```
- **Impact:** Classement de file moins précis
- **Fichier à modifier:** `Envy/EDClient.cpp` - Ajouter handler
- **Complexity:** Low (S)

**5. ED2K_C2C_FILEDESC**
- **Usage:** Description de fichier (commentaires, notes)
- **Format eMule:**
  ```
  FILEDESC: <Hash(16)><Rating(1)><CommentLength(2)><Comment...>
  ```
- **Impact:** Pas de partage de descriptions de fichiers
- **Fichier à modifier:** `Envy/EDClient.cpp` - Ajouter handler
- **Complexity:** Low (S)

**6. ED2K_C2C_REQUESTSOURCES / ED2K_C2C_ANSWERSOURCES**
- **Usage:** Demande/réponse de sources supplémentaires
- **Format eMule:**
  ```
  REQUESTSOURCES: <Hash(16)><SourceCount(1)>
  ANSWERSOURCES: <Hash(16)><SourceCount(1)><Sources...>
  ```
- **Impact:** Découverte de sources limitée
- **File to modify:** `Envy/EDClient.cpp` - Add handlers
- **Complexity:** Medium (M)

**7. ED2K_C2C_REQUESTPREVIEW / ED2K_C2C_PREVIEWANWSER**
- **Usage:** Prévisualisation de fichiers (images, etc.)
- **Format eMule:**
  ```
  REQUESTPREVIEW: <Hash(16)><PreviewType(1)>
  PREVIEWANWSER: <Hash(16)><PreviewType(1)><PreviewData...>
  ```
- **Impact:** Pas de prévisualisation de fichiers
- **File to modify:** `Envy/EDClient.cpp` - Add handlers
- **Complexity:** Medium (M)

---

## 🔴 Machines à États Manquantes

### 1. État de Connexion Kademlia

**eMule:** États détaillés pour chaque contact
- `KADEMLIA_UNKNOWN` - Contact non vérifié
- `KADEMLIA_BOOTSTRAP` - En cours de bootstrap
- `KADEMLIA_CONNECTING` - Connexion en cours
- `KADEMLIA_CONNECTED` - Connecté
- `KADEMLIA_DISCONNECTED` - Déconnecté
- `KADEMLIA_FIREWALLED` - Firewalled

**Envy:** État simple (`verified` booléen) - `Envy/Kademlia.h:51`

**Impact:** Gestion de contacts moins robuste

**Fichier à modifier:**
- `Envy/Kademlia.h` - Ajouter enum `KadContactState`
- `Envy/Kademlia.cpp` - Implémenter transitions d'état

**Complexity:** Low (S)

---

### 2. Timeouts et Retry Logic

**eMule:** Timeouts configurables par type de requête
- Bootstrap: 10s
- Ping: 5s
- Find Node: 5s
- Publish: 10s
- Retry: 3 tentatives avec backoff exponentiel

**Envy:** Timeout unique `KAD2_REQUEST_TIMEOUT = 30000` - `Envy/Kademlia.h:39`

**Impact:** Performance et résilience réduites

**Fichier à modifier:**
- `Envy/Kademlia.h` - Add specific timeouts
- `Envy/Kademlia.cpp:772-783` - Implement retry logic

**Complexity:** Low (S)

---

## 🔴 Comportements de Table de Routage

### 1. Bucket Splitting

**eMule:** Division de buckets quand pleins (si bucket contient notre ID)

**Envy:** Pas de splitting - `Envy/Kademlia.cpp:97-123` (ajout simple)

**Impact:** Table de routage moins optimale

**Fichier à modifier:**
- `Envy/Kademlia.h` - Add splitting logic
- `Envy/Kademlia.cpp` - Implement `SplitBucket()`

**Complexity:** Medium (M)

---

### 2. Contact Replacement (LRU)

**eMule:** Remplacement LRU quand bucket plein

**Envy:** Rejet si bucket plein - `Envy/Kademlia.cpp:56-61`

**Impact:** Contacts récents peuvent être perdus

**Fichier à modifier:**
- `Envy/Kademlia.cpp:44-62` - Implement LRU replacement

**Complexity:** Low (S)

---

### 3. Bucket Refresh

**eMule:** Refresh périodique des buckets (15 minutes)

**Envy:** Pas de refresh automatique

**Impact:** Buckets peuvent devenir obsolètes

**Fichier à modifier:**
- `Envy/Kademlia.cpp:358-376` - Add refresh in `OnTimer()`

**Complexity:** Low (S)

---

## 🔴 Primitives de Sécurité Manquantes

### 1. Validation d'ID de Nœud

**eMule:** Vérifie que les IDs de nœud sont valides (non-zéro, non-all-ones)

**Envy:** Vérification basique - `Envy/Kademlia.cpp:697-699` (zero check uniquement)

**Impact:** Nœuds malveillants peuvent être acceptés

**Fichier à modifier:**
- `Envy/Kademlia.cpp:690-717` - Add complete validation

**Complexity:** Low (S)

---

### 2. Protection contre Éclipse Attack

**eMule:** Limite le nombre de contacts depuis même /24 subnet

**Envy:** Pas de protection

**Impact:** Vulnérable aux attaques Eclipse

**Fichier à modifier:**
- `Envy/Kademlia.cpp:690-717` - Add subnet verification

**Complexity:** Low (S)

---

### 3. Challenge-Response pour Publish

**eMule:** Challenge cryptographique pour vérifier ownership de publish

**Envy:** Pas de challenge

**Impact:** Publish non sécurisé (spam possible)

**Fichier à modifier:**
- `Envy/Kademlia.cpp` - Ajouter challenge-response

**Complexity:** Medium (M)

---

## 🔴 NAT Traversal Manquants

### 1. UDP Hole Punching

**eMule:** Utilise hole punching pour contacter nœuds firewalled

**Envy:** Pas implémenté

**Impact:** Nœuds firewalled non contactables

**Fichier à modifier:**
- `Envy/Kademlia.cpp` - Add hole punching logic

**Complexity:** Medium-High (M-L)

---

### 2. Firewall Check (KAD)

**eMule:** Vérifie statut firewall via Kademlia

**Envy:** Pas implémenté

**Impact:** Statut firewall inconnu

**Fichier à modifier:**
- `Envy/Kademlia.cpp` - Add firewall check

**Complexity:** Medium (M)

---

## 📊 Plan d'Implémentation

### Phase 1: Messages Kademlia Essentiels (2 semaines)
1. **KADEMLIA2_HELLO_REQ/RES** - Semaine 1
2. **KADEMLIA2_REQ (FIND_VALUE)** - Semaine 1
3. **KADEMLIA2_REQ (STORE)** - Semaine 2
4. **KADEMLIA2_PUBLISH_REQ/RES** - Semaine 2

**Files:**
- `Envy/Kademlia.cpp` - Add handlers
- `Envy/Kademlia.h` - Add constants
- `Envy/EDPacket.h` - Vérifier opcodes

**Tests:**
- Interopérabilité avec eMule
- Recherche de fichiers fonctionnelle
- Publication de fichiers fonctionnelle

---

### Phase 2: Messages ED2K Avancés (1-2 semaines)
1. **ED2K_C2C_SECIDENTSTATE/SIGNATURE** - Corriger RNG/hash
2. **ED2K_C2C_EMULEINFO/ANSWER** - Semaine 1
3. **ED2K_C2C_COMPRESSEDPART** - Semaine 2

**Files:**
- `Envy/EDClient.cpp:64-159` - Fix SecureID
- `Envy/EDClient.cpp` - Add handlers
- `Envy/UploadTransferED2K.cpp` - Add compression

**Tests:**
- SecureID authentication functional
- eMule compatibility verified
- Compression tested

---

### Phase 3: Améliorations Table de Routage (1 semaine)
1. **Bucket Splitting** - 2-3 jours
2. **LRU Replacement** - 1-2 jours
3. **Bucket Refresh** - 1-2 jours

**Files:**
- `Envy/Kademlia.cpp` - Implement logics
- `Envy/Kademlia.h` - Add structures

**Tests:**
- Optimal routing table
- Contacts maintenus correctement

---

### Phase 4: Sécurité et NAT Traversal (2-3 semaines)
1. **Validation ID de Nœud** - 1 jour
2. **Protection Eclipse** - 2-3 jours
3. **Firewall Check** - 1 semaine
4. **UDP Hole Punching** - 1 semaine

**Files:**
- `Envy/Kademlia.cpp` - Add validations
- `Envy/Kademlia.cpp` - Implement NAT traversal

**Tests:**
- Résistance aux attaques
- NAT traversal fonctionnel

---

## 🧪 Harness de Test Minimal

```cpp
// tests/KademliaTest.cpp
class KademliaTest {
    // Test 1: Bootstrap
    void TestBootstrap() {
        // Envoyer BOOTSTRAP_REQ à nœud eMule
        // Vérifier BOOTSTRAP_RES reçu
        // Vérifier contacts ajoutés à table de routage
    }

    // Test 2: Find Node
    void TestFindNode() {
        // Envoyer FIND_NODE_REQ
        // Vérifier FIND_NODE_RES avec contacts proches
    }

    // Test 3: Find Value (fichier)
    void TestFindValue() {
        // Envoyer FIND_VALUE_REQ pour hash de fichier
        // Vérifier FIND_VALUE_RES avec sources
    }

    // Test 4: Publish
    void TestPublish() {
        // Publier fichier dans DHT
        // Vérifier PUBLISH_RES
    }

    // Test 5: Interopérabilité eMule
    void TestEMuleInterop() {
        // Connecter à nœud eMule réel
        // Vérifier échange de messages
        // Vérifier compatibilité
    }
};
```

---

## 📈 Métriques de Succès

- [ ] Tous les messages Kademlia essentiels implémentés
- [ ] Interopérabilité eMule vérifiée (tests réels)
- [ ] Recherche de fichiers fonctionnelle
- [ ] Publication de fichiers fonctionnelle
- [ ] Table de routage optimale (contacts maintenus)
- [ ] NAT traversal fonctionnel
- [ ] Sécurité: Résistance aux attaques Eclipse

---

## 📝 Notes de Compatibilité

### Format de Paquet
- **Endianness:** eMule utilise little-endian pour la plupart des champs
- **IP Address:** eMule stocke IP en host order (little-endian) dans payload Kademlia
- **Ports:** Network byte order (big-endian) pour ports UDP/TCP

### Versions
- **Kad Version:** eMule utilise version 8+ (actuellement)
- **Compatibilité:** Envy doit supporter version 8 pour interopérabilité

### Timeouts
- **Bootstrap:** 10s (eMule standard)
- **Ping:** 5s
- **Find Node:** 5s
- **Publish:** 10s

---

**Next Steps:**
1. Implémenter Phase 1 (messages essentiels)
2. Tester interopérabilité avec eMule
3. Itérer sur phases suivantes
4. Documenter différences restantes
