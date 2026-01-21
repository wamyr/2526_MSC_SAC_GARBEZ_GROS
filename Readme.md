## Commande MCC :
### Configuration :
  Les bras de pont U et V sont reliés aux pins PA8, PB13 (U), PA9 et PB14 (V). Ces pins sont configurées sur le Timer 1, channels 1 et 2. Nous allons générer des PWM complémentaires décalées permettant d’obtenir une tension moyenne nulle pour un rapport cyclique de 50 %. Ainsi, comparé à une commande bipolaire, on génère moins d’ondulation de courant et donc un bruit acoustique plus faible.
<<<<<<< HEAD
=======

Nous allons donc configurer le timer afin de respecter le cahier des charges. Pour obtenir une PWM à 20 kHz, on choisit un ARR le plus grand possible afin d’avoir une résolution maximale, et donc un prescaler le plus faible possible. On obtient alors : PSC = 0 (soit PSC = 1-1) et ARR = 8500-1, ce qui donne une fréquence de 20 kHz. Cette configuration offre une résolution sur 14 bits, ce qui respecte le cahier des charges

<img width="300" height="200" alt="image" src="https://github.com/user-attachments/assets/493a3abf-24ab-4cf4-8259-352fafc4517f" /> <img width="300" height="500" alt="image" src="https://github.com/user-attachments/assets/89a56ce6-4cd7-489e-bf92-1ef6649e4f25" /> <img width="300" height="200" alt="image" src="https://github.com/user-attachments/assets/5540c626-a5d7-4777-8607-ed3d958a3a1f" />

<img width="800" height="480" alt="tek00005" src="https://github.com/user-attachments/assets/aa0b6c5d-8a67-4769-ad03-d98fa638a8a7" />

On obtient bien le résultat attendu.

### Temps morts :
  Pour les temps morts, la datasheet des transistors indique un deadtime de 100 ns pour une clock de 170 MHz. Il faut donc 17 ticks pour générer un deadtime de 100 ns.

![WhatsApp Image 2026-01-19 at 19 21 59](https://github.com/user-attachments/assets/65b72cd0-cb1f-4779-8335-a1db0c78537f)

<img width="800" height="480" alt="tek00006" src="https://github.com/user-attachments/assets/5ec199bd-5b4b-49aa-83c8-a7bf817f15b5" />

On a obtient bien le résultat attendu.

### Commande par le shell :
Pour activer les PWM, nous avons écrit une fonction motor_start() qui prend en argument "start" ou "stop". Ces arguments activent ou désactivent respectivement les PWM du Timer 1. Si l’on souhaite alimenter le moteur, on commence par définir le rapport cyclique à 50 % afin d’éviter un démarrage involontaire du moteur.
Pour définir la vitesse du moteur, nous avons écrit une fonction motor_speed() qui prend en entrée le rapport cyclique souhaité. Cette valeur est limitée : dans le cas où l’on dépasse 0 % ou 100 %, la valeur précédente est conservée et un message d’erreur est affiché.

Ces fonctions sont utilisées dans le shell et sont initialisées dans la fonction motor_init()
```shell
> motor start
> speed 60
```

Suite à cette commande, on peut observer sur l'oscilloscope les PWMs complémentaires décallés comme précédemment. 

### Premier test :
Nous observons que le moteur tourne de manière cohérente en fonction du rapport cyclique. Cependant, certains problèmes apparaissent.
Tout d’abord, des pics de courant sont observés lorsque l’on augmente brusquement le rapport cyclique. Cela est dû à l’augmentation rapide de la tension appliquée au moteur, ce qui provoque un appel de courant important à cause de l’inductance et de l’inertie mécanique du système.

Pour pallier ce problème, nous avons mis en place une rampe permettant d’augmenter progressivement la tension aux bornes du moteur, et ainsi limiter ces pics de courant. Nous avons donc écrit la fonction motor_ramp_update() qui ajoute un incrément au duty cycle après une période d’attente, jusqu’à atteindre la valeur finale.
Cette période est réglée à l’aide du Timer 16. Elle n’a pas besoin d’être très précise, car l’objectif est simplement d’attendre un certain temps avant chaque incrément. En effet, étant donné que le programme fonctionne avec des interruptions dues au shell, il est plus difficile d’utiliser une fonction bloquante comme HAL_Delay(). Le signe de l’incrément s’adapte automatiquement selon que l’on augmente ou que l’on diminue le rapport cyclique.

## Mesure du courant :
La mesure du courant est réalisée à l’aide du capteur GO 10-SME. Ce capteur est un transducteur de courant à effet Hall. Il convertit le courant traversant le conducteur primaire ($I_P$) en une tension de sortie proportionnelle ($V_{out}$). À 0 A, le capteur délivre une tension de référence $V_{ref} = 2.5 V$. Sa sensibilité nominale est de 25 mV/A. On peut donc mesurer le courant des phases à partir de la tension de sortie du capteur :

Pour lire cette tension, nous allons utilisé un adc, celui de la pin de la nucleo reliée à Vout. Pour cela on va trigger l'adc sur le timer de la PWM du bras de pont afin d'avoir un un courant "propre". En effet, nous allons mesurer au moment où le courant est le plus lisse c'est à dire au milieu de la période. 

Pour ne pas surcharger le CPU mais tout de même mesurer en continu nous allons utiliser un DMA en mode circular. Lorsque l'on demande la valeur du courant par exemple si on appelle la fonction "cmd_current()" dans le shell, on récupère les 10 dernières valeurs de courant pour obtenir un courant moyen : adc_raw, puis on convertir en numérique :
$$V_{mesuré} = adc_{raw} \times \frac{V_{DDA}}{4095}$$ Où $V_{DDA}$ est la tension d'alimentation analogique du microcontrôleur (3.3V).
Puis on récupère la valeur du courant :
$$I_{réel} = \frac{V_{mesuré} - V_{offset}}{Sensibilité}$$
Ces étapes sont réalisées dans la fonction "Calculer_Courant_Moyen()"

```shell
> current get
```

### Résultat : 

<img width="304" height="165" alt="image" src="https://github.com/user-attachments/assets/57b4bcdf-b108-4c3c-ab35-8ca2e3f14726" />

<img width="800" height="480" alt="tek00002" src="https://github.com/user-attachments/assets/14a68196-3259-46d2-86cc-0cdad694a188" />

On observe que le courant est cohérent lorsque le moteur tourne. On se rendra compte plus tard que l'on aurait dû faire plus de test afin d'être sûr de la calibration du capteur. 

## Mesure de la vitesse :
On va utiliser l'encodeur incrémental présent sur la maquette pour mesurer la vitesse. On affiche avec la sonde numérique les sorties A, B et Z de l'encodeur sur l'oscilloscope. On en sort la fréquence d'un channel A/B et du channel z. Le gain de l'encodeur (fonction de transfert) est donc 
$$\frac{F_{channelA}}{F_{channelZ}}$$:

<img width="800" height="480" alt="tek00001" src="https://github.com/user-attachments/assets/c3e944d1-3f2d-4644-bb22-0b05e470222f" />


<img width="800" height="480" alt="tek00000" src="https://github.com/user-attachments/assets/8d9394ee-5061-4589-91cc-839dde864c9d" />

### Résultats :

On obtient un gain d'environ 1016, par définition du capteur, celui-ci ne peut qu'avoir un gain équivalent à une puissance de 2. La puissance de 2 la plus proche étant 1024, le gain est 2^10.

## PID

Pour le PID on a déjà besoin de déterminer comment l’implémenter dans le code actuel. Il faut qu’il asservisse en boucle donc on mettra évidemment le code dans la loop. La fonction speed déclenche juste un “flag” permettant de commencer le PID et d’appliquer la consigne de vitesse. Dans la loop en plus du flags qui permet de commencer l’asservissement il faut aussi contrôler l’asservissement de courant et de vitesse pour qu’elle ne s’exécute que selon une période souhaitée. Pour le courant la fréquence d’asservissement s’aligne avec la fréquence de la PWM pour mesurer au bon moment le courant et réduire le bruit de commutation. Pour la vitesse le temps d’échantillonnage a été défini à 100ms lors du réglages des coefficients du PI.
Les deux correcteurs PI ont une limitation de sortie conforme aux caractéristiques du moteur, ainsi qu'une limite anti windup au niveau correcteur intégral. 
De plus pour réaliser une correction correcte avec notre système discret, le PI est transformé dans le domaine discret selon la transformation bilinéaire. 
Calcul de la sortie en fonction de l'erreur, de l'ancienne sortie et de l'ancienne erreur : 

<img width="1497" height="726" alt="image" src="https://github.com/user-attachments/assets/d139c829-5e83-4fd8-9f27-68e70d2488ac" />



On a bouclé le système suivant le selon le schéma suivant :

<img width="1844" height="350" alt="image" src="https://github.com/user-attachments/assets/4a264f1f-2fd6-4beb-8a79-14bfa6a2a0f0" />
Version pdf : [MSC_SAC_Asserv-schemablock.pdf](https://github.com/user-attachments/files/24754202/MSC_SAC_Asserv-schemablock.pdf)

On s'est basé sur ce schéma pour faire le PI. Cependant le modèle qui nous a été fourni duquel a été déterminé les coefficients n'est pas tout à fait pareil. En effet, en sorti de l'asservissement de courant on obtenait le rapport cyclique. Cela explique peut-être pourquoi le correcteur de courant ne fonctionnait pas et saturait à 1V. 

## Résultat :
### Observation :
On a testé en alimentant le moteur à Vcc. On a observé que le moteur ne tournait pas, du moins pas à la consigne demandé. On a donc décidé de débugger en envoyant dans le shell les valeurs que renvoyé les PID notamment celui de courant pour commencer. On s'est rendu compte que le PID de courant ne renvoyait pas de consigne cohérente et celles ci finissaient pas stagner à une valeure trop basse par rapport à ce qui était attendu.
On a donc affiché dans le terminal les mesures de courant renvoyées par le capteur. Ces valeurs ne correspondaient pas du tout à ce que le générateur affiché pouvant conclure sur au moins une erreur de la mesure. 

Pour vérifier l'écriture du PID, on aurait pu également tester celui de vitesse car la mesure de vitesse était correct ce qui aurait pu isoler le problème de la mesure de courant. 

### Conclusion :
Nous avons réussit à controler un moteur Mcc par une commande de PWM complémentaire décalée. Pour cela, on a réussit à utiliser le shell à notre disposition en écrivant des fonctions liant commande shell et commande moteur. Nous avons su récupérer les composants (transistors, capteurs) à partir de la documentation et utilisait leurs datasheet et le cours à disposition pour mesurer des grandeurs du système (vitesse, courant).
A partir des simulations lors des Travaux dirigés, nous avons pu récupérer les coefficients du PID discret et implémenter un code permettant d'asservir le moteur. Il nous aura manqué un peu de temps pour debbug la mesure de courant et donc l'asservissement en boucle fermé du système. 
On aurait surement dû ajouter une calibration du capteur de courant car comme expliqué dans la datasheet, il y a effectivement la possibilité que la sensibilité ne soit pas exactement celle annoncée.




>>>>>>> 8ea26cf388300ab0f7f666418382111efeff56e4

Nous allons donc configurer le timer afin de respecter le cahier des charges. Pour obtenir une PWM à 20 kHz, on choisit un ARR le plus grand possible afin d’avoir une résolution maximale, et donc un prescaler le plus faible possible. On obtient alors : PSC = 0 (soit PSC = 1-1) et ARR = 8500-1, ce qui donne une fréquence de 20 kHz. Cette configuration offre une résolution sur 14 bits, ce qui respecte le cahier des charges

<img width="300" height="200" alt="image" src="https://github.com/user-attachments/assets/493a3abf-24ab-4cf4-8259-352fafc4517f" /> <img width="300" height="500" alt="image" src="https://github.com/user-attachments/assets/89a56ce6-4cd7-489e-bf92-1ef6649e4f25" /> <img width="300" height="200" alt="image" src="https://github.com/user-attachments/assets/5540c626-a5d7-4777-8607-ed3d958a3a1f" />

<img width="800" height="480" alt="tek00005" src="https://github.com/user-attachments/assets/aa0b6c5d-8a67-4769-ad03-d98fa638a8a7" />

On obtient bien le résultat attendu.

### Temps morts :
  Pour les temps morts, la datasheet des transistors indique un deadtime de 100 ns pour une clock de 170 MHz. Il faut donc 17 ticks pour générer un deadtime de 100 ns.

![WhatsApp Image 2026-01-19 at 19 21 59](https://github.com/user-attachments/assets/65b72cd0-cb1f-4779-8335-a1db0c78537f)

<img width="800" height="480" alt="tek00006" src="https://github.com/user-attachments/assets/5ec199bd-5b4b-49aa-83c8-a7bf817f15b5" />

On a obtient bien le résultat attendu.

### Commande par le shell :
Pour activer les PWM, nous avons écrit une fonction motor_start() qui prend en argument "start" ou "stop". Ces arguments activent ou désactivent respectivement les PWM du Timer 1. Si l’on souhaite alimenter le moteur, on commence par définir le rapport cyclique à 50 % afin d’éviter un démarrage involontaire du moteur.
Pour définir la vitesse du moteur, nous avons écrit une fonction motor_speed() qui prend en entrée le rapport cyclique souhaité. Cette valeur est limitée : dans le cas où l’on dépasse 0 % ou 100 %, la valeur précédente est conservée et un message d’erreur est affiché.

Ces fonctions sont utilisées dans le shell et sont initialisées dans la fonction motor_init()
```shell
> motor start
> speed 60
```

Suite à cette commande, on peut observer sur l'oscilloscope les PWMs complémentaires décallés comme précédemment. 

### Premier test :
Nous observons que le moteur tourne de manière cohérente en fonction du rapport cyclique. Cependant, certains problèmes apparaissent.
Tout d’abord, des pics de courant sont observés lorsque l’on augmente brusquement le rapport cyclique. Cela est dû à l’augmentation rapide de la tension appliquée au moteur, ce qui provoque un appel de courant important à cause de l’inductance et de l’inertie mécanique du système.

Pour pallier ce problème, nous avons mis en place une rampe permettant d’augmenter progressivement la tension aux bornes du moteur, et ainsi limiter ces pics de courant. Nous avons donc écrit la fonction motor_ramp_update() qui ajoute un incrément au duty cycle après une période d’attente, jusqu’à atteindre la valeur finale.
Cette période est réglée à l’aide du Timer 16. Elle n’a pas besoin d’être très précise, car l’objectif est simplement d’attendre un certain temps avant chaque incrément. En effet, étant donné que le programme fonctionne avec des interruptions dues au shell, il est plus difficile d’utiliser une fonction bloquante comme HAL_Delay(). Le signe de l’incrément s’adapte automatiquement selon que l’on augmente ou que l’on diminue le rapport cyclique.

## Mesure du courant :
La mesure du courant est réalisée à l’aide du capteur GO 10-SME. Ce capteur est un transducteur de courant à effet Hall. Il convertit le courant traversant le conducteur primaire ($I_P$) en une tension de sortie proportionnelle ($V_{out}$). À 0 A, le capteur délivre une tension de référence $V_{ref} = 2.5 V$. Sa sensibilité nominale est de 25 mV/A. On peut donc mesurer le courant des phases à partir de la tension de sortie du capteur :

Pour lire cette tension, nous allons utilisé un adc, celui de la pin de la nucleo reliée à Vout. Pour cela on va trigger l'adc sur le timer de la PWM du bras de pont afin d'avoir un un courant "propre". En effet, nous allons mesurer au moment où le courant est le plus lisse c'est à dire au milieu de la période. 

Pour ne pas surcharger le CPU mais tout de même mesurer en continu nous allons utiliser un DMA en mode circular. Lorsque l'on demande la valeur du courant par exemple si on appelle la fonction "cmd_current()" dans le shell, on récupère les 10 dernières valeurs de courant pour obtenir un courant moyen : adc_raw, puis on convertir en numérique :
$$V_{mesuré} = adc_{raw} \times \frac{V_{DDA}}{4095}$$ Où $V_{DDA}$ est la tension d'alimentation analogique du microcontrôleur (3.3V).
Puis on récupère la valeur du courant :
$$I_{réel} = \frac{V_{mesuré} - V_{offset}}{Sensibilité}$$
Ces étapes sont réalisées dans la fonction "Calculer_Courant_Moyen()"

```shell
> current get
```

### Résultat : 

<img width="304" height="165" alt="image" src="https://github.com/user-attachments/assets/57b4bcdf-b108-4c3c-ab35-8ca2e3f14726" />

<img width="800" height="480" alt="tek00002" src="https://github.com/user-attachments/assets/14a68196-3259-46d2-86cc-0cdad694a188" />

On observe que le courant est cohérent lorsque le moteur tourne. On se rendra compte plus tard que l'on aurait dû faire plus de test afin d'être sûr de la calibration du capteur. 

## Mesure de la vitesse :
On va utiliser l'encodeur incrémental présent sur la maquette pour mesurer la vitesse. On affiche avec la sonde numérique les sorties A, B et Z de l'encodeur sur l'oscilloscope. On en sort la fréquence d'un channel A/B et du channel z. Le gain de l'encodeur (fonction de transfert) est donc 
$$\frac{F_{channelA}}{F_{channelZ}}$$:

<img width="800" height="480" alt="tek00001" src="https://github.com/user-attachments/assets/c3e944d1-3f2d-4644-bb22-0b05e470222f" />


<img width="800" height="480" alt="tek00000" src="https://github.com/user-attachments/assets/8d9394ee-5061-4589-91cc-839dde864c9d" />

### Résultats :

On obtient un gain d'environ 1016, par définition du capteur, celui-ci ne peut qu'avoir un gain équivalent à une puissance de 2. La puissance de 2 la plus proche étant 1024, le gain est 2^10.

## PID

Pour le PID on a déjà besoin de déterminer comment l’implémenter dans le code actuel. Il faut qu’il asservisse en boucle donc on mettra évidemment le code dans la loop. La fonction speed déclenche juste un “flag” permettant de commencer le PID et d’appliquer la consigne de vitesse. Dans la loop en plus du flags qui permet de commencer l’asservissement il faut aussi contrôler l’asservissement de courant et de vitesse pour qu’elle ne s’exécute que selon une période souhaitée. Pour le courant la fréquence d’asservissement s’aligne avec la fréquence de la PWM pour mesurer au bon moment le courant et réduire le bruit de commutation. Pour la vitesse le temps d’échantillonnage a été défini à 100ms lors du réglages des coefficients du PI.
Les deux correcteurs PI ont une limitation de sortie conforme aux caractéristiques du moteur, ainsi qu'une limite anti windup au niveau correcteur intégral. 
De plus pour réaliser une correction correcte avec notre système discret, le PI est transformé dans le domaine discret selon la transformation bilinéaire. 
Calcul de la sortie en fonction de l'erreur, de l'ancienne sortie et de l'ancienne erreur : 

<img width="1497" height="726" alt="image" src="https://github.com/user-attachments/assets/d139c829-5e83-4fd8-9f27-68e70d2488ac" />



On a bouclé le système suivant le selon le schéma suivant :

<img width="1844" height="350" alt="image" src="https://github.com/user-attachments/assets/4a264f1f-2fd6-4beb-8a79-14bfa6a2a0f0" />
Version pdf : [MSC_SAC_Asserv-schemablock.pdf](https://github.com/user-attachments/files/24754202/MSC_SAC_Asserv-schemablock.pdf)

On s'est basé sur ce schéma pour faire le PI. Cependant le modèle qui nous a été fourni duquel a été déterminé les coefficients n'est pas tout à fait pareil. En effet, en sorti de l'asservissement de courant on obtenait le rapport cyclique. Cela explique peut-être pourquoi le correcteur de courant ne fonctionnait pas et saturait à 1V. 

## Résultat :
### Observation :
On a testé en alimentant le moteur à Vcc. On a observé que le moteur ne tournait pas, du moins pas à la consigne demandé. On a donc décidé de débugger en envoyant dans le shell les valeurs que renvoyé les PID notamment celui de courant pour commencer. On s'est rendu compte que le PID de courant ne renvoyait pas de consigne cohérente et celles ci finissaient pas stagner à une valeure trop basse par rapport à ce qui était attendu.
On a donc affiché dans le terminal les mesures de courant renvoyées par le capteur. Ces valeurs ne correspondaient pas du tout à ce que le générateur affiché pouvant conclure sur au moins une erreur de la mesure. 

Pour vérifier l'écriture du PID, on aurait pu également tester celui de vitesse car la mesure de vitesse était correct ce qui aurait pu isoler le problème de la mesure de courant. 

### Conclusion :
Nous avons réussit à controler un moteur Mcc par une commande de PWM complémentaire décalée. Pour cela, on a réussit à utiliser le shell à notre disposition en écrivant des fonctions liant commande shell et commande moteur. Nous avons su récupérer les composants (transistors, capteurs) à partir de la documentation et utilisait leurs datasheet et le cours à disposition pour mesurer des grandeurs du système (vitesse, courant).
A partir des simulations lors des Travaux dirigés, nous avons pu récupérer les coefficients du PID discret et implémenter un code permettant d'asservir le moteur. Il nous aura manqué un peu de temps pour debbug la mesure de courant et donc l'asservissement en boucle fermé du système. 
On aurait surement dû ajouter une calibration du capteur de courant car comme expliqué dans la datasheet, il y a effectivement la possibilité que la sensibilité ne soit pas exactement celle annoncée.