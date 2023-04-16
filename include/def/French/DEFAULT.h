#include "labeldef.h"
//  This file is a definition file for the ESPAtherma.
//  uncomment each value you want to query for your installation.

LabelDef labelDefs[] = {
//{0x00,0,801,0,-1,"*Type de réfrigérant"},
//{0x00,0,152,1,-1,"Qté de données du capteur"},
//{0x00,1,152,1,-1,"Qté compresseur INV"},
//{0x00,2,152,1,-1,"Compresseur STD"},
//{0x00,3,152,1,-1,"Qté de données de ventilateur"},
//{0x00,4,152,1,-1,"Qté de données de détendeur électronique"},
//{0x00,5,152,1,-1,"Qté de données de vanne à 4 voies"},
//{0x00,6,152,1,-1,"Qté de résistance de carter"},
//{0x00,7,152,1,-1,"Qté électrovanne"},
//{0x00,8,152,1,-1,"Unités intérieures connectables max."},
//{0x00,9,152,1,-1,"Qté unité intérieure connectée"},
//{0x00,10,213,1,-1,"O/U MPU ID"},
//{0x00,11,213,1,-1,"O/U MPU ID"},
//{0x10,0,217,1,-1,"Mode de fonctionnement"},
//{0x10,1,307,1,-1,"Thermostat ON/OFF"},
//{0x10,1,306,1,-1,"Attente de redémarrage"},
//{0x10,1,305,1,-1,"Commande de démarrage"},
//{0x10,1,304,1,-1,"Dégivrage"},
//{0x10,1,303,1,-1,"Commande de retour d huile"},
//{0x10,1,302,1,-1,"Opération d égalisation de pression"},
//{0x10,1,301,1,-1,"Signal de demande"},
//{0x10,1,300,1,-1,"Contrôle de bruit réduit"},
//{0x10,2,307,1,-1,"Autre contrôle1"},
//{0x10,2,306,1,-1,"Autre contrôle2"},
//{0x10,2,305,1,-1,"Autre contrôle3"},
//{0x10,2,304,1,-1,"Autre contrôle4"},
//{0x10,2,303,1,-1,"Autre contrôle5"},
//{0x10,2,302,1,-1,"Autre contrôle6"},
//{0x10,2,301,1,-1,"Autre contrôle7"},
//{0x10,2,300,1,-1,"Autre contrôle8"},
//{0x10,3,307,1,-1,"Autre contrôle9"},
//{0x10,3,306,1,-1,"Autre contrôle10"},
//{0x10,3,305,1,-1,"Autre contrôle11"},
//{0x10,3,304,1,-1,"Autre contrôle12"},
//{0x10,3,303,1,-1,"Autre contrôle13"},
//{0x10,3,302,1,-1,"Autre contrôle14"},
//{0x10,3,301,1,-1,"Autre contrôle15"},
//{0x10,3,300,1,-1,"Autre contrôle16"},
//{0x10,4,203,1,-1,"Type de dysfonctionnement"},
//{0x10,5,204,1,-1,"Code de dysfonctionnement"},
//{0x10,6,114,2,1,"Temp. évap. cible"},
//{0x10,8,114,2,1,"Temp. cond. cible"},
//{0x10,10,307,1,-1,"Chute de temp. refoulement"},
//{0x10,10,310,1,-1,"Qté réessai de protection de temp. refoulement"},
//{0x10,10,303,1,-1,"Chute de courant INV comp."},
//{0x10,10,311,1,-1,"Qté réessai de protection de courant INV. comp."},
//{0x10,11,307,1,-1,"Contrôle de chute de HP"},
//{0x10,11,310,1,-1,"Qté réessai de protection HP"},
//{0x10,11,303,1,-1,"Contrôle de chute de BP"},
//{0x10,11,311,1,-1,"Qté de réessai de protection de BP"},
//{0x10,12,303,1,-1,"Autre contrôle"},
//{0x00,0,995,1,-1,"NextDataGrid"},
//{0x20,0,105,2,1,"Temp. air extérieur"},
//{0x20,2,105,2,1,"Temp. échangeur de chaleur extérieur"},
//{0x20,4,105,2,1,"Temp. tuyau refoulement"},
//{0x20,6,105,2,1,"Température du tuyau d aspiration"},
//{0x20,8,105,2,1,"Temp. intermédiaire échangeur de chaleur extérieur"},
//{0x20,10,105,2,1,"Temp. tuyau de liquide"},
//{0x20,12,105,2,1,"Temp. ailette INV"},
//{0x20,14,105,2,2,"Basse pression"},
//{0x20,14,405,2,1,"Basse pression(T)"},
//{0x20,16,105,2,-1,"Données du capteur 9"},
//{0x20,18,105,2,-1,"Données du capteur 10"},
//{0x20,20,105,2,-1,"Données du capteur 11"},
//{0x21,0,105,2,-1,"Courant primaire INV (A)"},
//{0x21,2,105,2,-1,"Courant secondaire INV (A)"},
//{0x21,4,105,2,1,"Temp. ailette INV"},
//{0x21,6,105,2,-1,"Données du capteur 4"},
//{0x21,8,105,2,-1,"Données du capteur 5"},
//{0x21,10,105,2,-1,"Données du capteur 6"},
//{0x21,12,105,2,-1,"Données du capteur 7"},
//{0x21,14,105,2,-1,"Données du capteur 8"},
//{0x21,16,105,2,-1,"Données du capteur 9"},
//{0x21,18,105,2,-1,"Données du capteur 10"},
//{0x21,20,105,2,-1,"Données du capteur 11"},
//{0x00,0,995,1,-1,"NextDataGrid"},
//{0x30,0,152,1,-1,"Fréquence INV 1 (rps)"},
//{0x30,1,152,1,-1,"Fréquence INV 2 (rps)"},
//{0x30,0,307,1,-1,"Compresseur STD 1"},
//{0x30,0,306,1,-1,"Compresseur STD 2"},
//{0x30,0,211,1,-1,"Ventilateur 1 (palier)"},
//{0x30,1,211,1,-1,"Ventilateur 2 (palier)"},
//{0x30,0,151,2,-1,"Détendeur électronique1 (pls)"},
//{0x30,2,151,2,-1,"Détendeur électronique2 (pls)"},
//{0x30,0,307,1,-1,"Vanne à 4 voies 1"},
//{0x30,0,306,1,-1,"Vanne à 4 voies 2"},
//{0x30,0,307,1,-1,"Résistance de carter 1"},
//{0x30,0,306,1,-1,"Résistance de carter 2"},
//{0x30,0,307,1,-1,"Electrovanne 1"},
//{0x30,0,306,1,-1,"Electrovanne 2"},
//{0x30,0,305,1,-1,"Electrovanne 3"},
//{0x30,0,304,1,-1,"Electrovanne 4"},
//{0x30,0,303,1,-1,"Electrovanne 5"},
//{0x30,0,302,1,-1,"Electrovanne 6"},
//{0x30,0,301,1,-1,"Electrovanne 7"},
//{0x30,0,300,1,-1,"Electrovanne 8"},
//{0x30,1,307,1,-1,"Electrovanne 9"},
//{0x30,1,306,1,-1,"Electrovanne 10"},
//{0x00,0,998,1,-1,"In-Out separator"},
//{0x60,0,307,1,-1,"Activer/Désactiver les données"},
//{0x60,1,152,1,-1,"Adresse d unité intérieure"},
//{0x60,2,315,1,-1,"Mode de fonctionnement intérieur"},
//{0x60,2,303,1,-1,"Thermostat ON/OFF"},
//{0x60,2,302,1,-1,"Protection antigel"},
//{0x60,2,301,1,-1,"Mode silencieux"},
//{0x60,2,300,1,-1,"Protection antigel pour tuyauterie eau"},
//{0x60,3,204,1,-1,"Code de dysfonctionnement"},
//{0x60,4,314,2,-1,"Code d unité intérieure"},
//{0x60,6,219,1,-1,"Capacité d unité intérieure"},
//{0x60,7,105,2,1,"Point de réglage de télécommande"},
//{0x60,9,105,2,1,"Point de réglage de sortie H/P"},
//{0x60,11,307,1,-1,"Interrupteur de débit"},
//{0x60,11,306,1,-1,"Clixon BUH"},
//{0x60,11,305,1,-1,"Clixon BSH"},
//{0x60,11,304,1,-1,"Entrée du tarif HP"},
//{0x60,11,303,1,-1,"Entrée solaire"},
//{0x60,11,302,1,-1,"Non utilisé"},
//{0x60,11,301,1,-1,"Non utilisé"},
//{0x60,11,300,1,-1,"Fonctionnement bivalent"},
//{0x60,12,307,1,-1,"Vanne à 3 voies (Froid/Chaud)"},
//{0x60,12,306,1,-1,"Vanne à 4 voies (Espace/DHW)"},
//{0x60,12,305,1,-1,"BSH"},
//{0x60,12,304,1,-1,"BUH1"},
//{0x60,12,303,1,-1,"BUH2"},
//{0x60,12,302,1,-1,"Chauffage de plaque inférieure"},
//{0x60,12,301,1,-1,"Puissance de pompe"},
//{0x60,12,300,1,-1,"Signal de sortie solaire"},
//{0x60,13,212,1,-1,"Code d option intérieur"},
//{0x60,14,314,2,-1,"Adresse de logiciel intérieure"},
//{0x60,15,212,1,-1,"Version EEPROM intérieure"},
//{0x61,0,307,1,-1,"Activer/Désactiver les données"},
//{0x61,1,152,1,-1,"Adresse d unité intérieure"},
//{0x61,2,105,2,1,"PHE de sortie(R1T)"},
//{0x61,4,105,2,1,"BUH de sortie(R2T)"},
//{0x61,6,105,2,1,"Température de liquide (R3T)"},
//{0x61,8,105,2,1,"Température d entrée (R4T)"},
//{0x61,10,105,2,1,"Température de DHW (R5T)"},
//{0x61,12,105,2,1,"Thermistor à télécommande"},
};
