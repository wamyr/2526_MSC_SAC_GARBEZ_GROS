## Commande MCC :
Les bras de pont U et V sont reliés aux pins PA8, Pb13(U), PA9 et PB14(V). Ces pins sont configurées sur le Timer1 channel 1 et 2. On va générer des PWMs complémentaire décalés qui ont pour permettent d'avoir une tension moyenne nulle au rapport cyclique 50%. Ainsi, comparé à une commande bipolaire, on génére moins d'ondulation de courant et donc  un plus faible bruint accoustique.  

On va donc configurer le Timer de manière à respecter le cahier des charges. Pour les 20KHz de la PWM, on va préférer choisir le ARR le plus grand possible pour avoir une résolution maximale et donc avoir un prescaler le plus faible possible. On se retrouve à PSC = 0-1, ARR = 8500-1 qui donne une fréquence de 20KHz. Cette configuration sera sur 14 bits qui respecte le cahier des charges.
<img width="477" height="271" alt="image" src="https://github.com/user-attachments/assets/493a3abf-24ab-4cf4-8259-352fafc4517f" />

<img width="464" height="194" alt="image" src="https://github.com/user-attachments/assets/89a56ce6-4cd7-489e-bf92-1ef6649e4f25" />

<img width="421" height="218" alt="image" src="https://github.com/user-attachments/assets/5540c626-a5d7-4777-8607-ed3d958a3a1f" />

Pour les temps morts, la datasheet des transistors donnent un deadtime de 100ns pour une clock de 170MHz. Il faut donc 17 ticks pour générer un deadtime de 100ns. 

![WhatsApp Image 2026-01-19 at 19 21 59](https://github.com/user-attachments/assets/65b72cd0-cb1f-4779-8335-a1db0c78537f)


Pour activer les PWMs, nous avons écrit une fonction "motor_start() qui prend en argument "start" ou "stop". Ces arguments activent ou désactives respectivement les PWMs du timer 1. Si on veut allimenter le moteur, on commence par définir le rapport cyclique à 50 pour éviter de démarrer le moteur sans le vouloir.
Pour définir la vitesse de celui-ci, nous avons écrit une fonction "motor_speed()" qui prend en entrée, dans un premier temps, le rapport cyclique souhaitée. Cette valeur étant limitée. Dans le cas où l'on dépasserait 0% ou 100%, on reste à la valeure précédente et onn affiche un message d'erreur.
Ces fonctions sont utilisées dans le shell et initié dans la fonction "motor init()". 
```shell
> motor start
> speed 60
```

(Photo à prendre du résultat)

Premier test :
Nous observons que le moteur tourne de manière cohérente selon les rapports cycliques. Cependant des problèmes apparaissent :
Tout d'abord, des pics de courant apparaissent lorsque l'on augmente brusquement le rapport cyclique. (expliquer pourquoi relou).

Pour palier à ce problème, (info Ousmane) nous allons réaliser une rampe qui permet d'augmenter progressivement la tension aux bornes du moteur et donc éviter ces fameux pics. On a donc écrit la fonction "motor_ramp_update()" qui ajoute un incrément au duty cycle pendant une période d'attente jusqu'à la valeur finale. La période, réglée avec le timer 16, n'est pas faites pour être très précise car le but étant d'attendre un certains temps avant d'incrémenter. En effet, étant en interruption à cause du shell c'est plus laborieux d'"attendre" avec un HAL_delay par exemple. Le signe de l'incrément s'adapte si on baisse le duty cycle ou si on l'augmente.


(Photo à prendre du résultat)

## Mesure du courant :
La mesure du courant se réalise par le capteur GO 10-SME. Ce capteur est un transducteur de courant à effet Hall. Il convertit le courant traversant le conducteur primaire ($I_P$) en une tension de sortie proportionnelle ($V_{out}$). A 0A, le capteur délibre la tension Vref = 2.5V. Sa sensibilité nominale est de 25mV/A. On peut donc mesurer le courant des phases à partir de la tension de sortie du capteur : $$V_{out} = V_{ref} \pm (S \times I_P)$$

Pour lire cette tension, nous allons utilisé un adc, celui de la pin de la nucleo reliée à Vout. Pour cela on va trigger l'adc sur le timer de la PWM du bras de pont afin d'avoir un un courant "propre". En effet, nous allons mesurer au moment où le courant est le plus lisse c'est à dire au milieu de la période. 

Pour ne pas surcharger le CPU mais tout de même mesurer en continu nous allons utiliser un DMA en mode circular. Lorsque l'on demande la valeur du courant par exemple si on appelle la fonction "cmd_current()" dans le shell, on récupère les 10 dernières valeurs de courant pour obtenir un courant moyen : adc_raw, puis on convertir en numérique :
$$V_{mesuré} = adc\_raw \times \frac{V_{DDA}}{4095}$$ Où $V_{DDA}$ est la tension d'alimentation analogique du microcontrôleur (3.3V).
Puis on récupère la valeur du courant :
$$I_{réel} = \frac{V_{mesuré} - V_{offset}}{Sensibilité}$$
Ces étapes sont réalisées dans la fonction "Calculer_Courant_Moyen()"

Résultat : 


## Mesure de la vitesse :

On affiche avec la sonde numérique les sorties A, B et Z de l'encodeur sur l'oscilloscope. On en sors la fréquence d'un channel A/B et du channel z. Le gain de l'encodeur est donc FchannelA/FchannelZ :

<img width="800" height="480" alt="tek00001" src="https://github.com/user-attachments/assets/c3e944d1-3f2d-4644-bb22-0b05e470222f" />


<img width="800" height="480" alt="tek00000" src="https://github.com/user-attachments/assets/8d9394ee-5061-4589-91cc-839dde864c9d" />

On obtient un gain d'environ 1016, la puissance de 10 la plus proche étant 1024, le gain est 2^10.

##PID

Pour le PID on a déjà besoin de déterminer comment l’implémenter dans le code actuel. Il faut qu’il asservisse en boucle donc on mettra évidemment le code dans la loop. La fonction speed déclenche juste un “flag” permettant de commencer le PID et d’appliquer la consigne de vitesse. Dans la loop en plus du flags qui permet de commencer l’asservissement il faut aussi contrôler l’asservissement de courant et de vitesse pour qu’elle ne s’exécute que selon une période souhaitée. Pour le courant la fréquence d’asservissement s’aligne avec la fréquence de la PWM pour mesurer au bon moment le courant et réduire le bruit de commutation. Pour la vitesse le temps d’échantillonnage a été défini à 100ms lors du réglages des coefficients du PI.


On a bouclé le système suivant le selon le schéma suivant :
##Résultat :
### Observation :
On a testé en alimentant le moteur à Vcc. On a observé que le moteur ne tournait pas, du moins pas à la consigne demandé. On a donc décidé de débugger en envoyant dans le shell les valeurs que renvoyé les PID notamment celui de courant pour commencer. On s'est rendu compte que le PID de courant ne renvoyait pas de consigne cohérente et celles ci finissaient pas stagner à une valeure trop basse par rapport à ce qui était attendu.
On a donc affiché dans le terminal les mesures de courant renvoyées par le capteur. Ces valeurs ne correspondaient pas du tout à ce que le générateur affiché pouvant conclure sur au moins une erreur de la mesure. 

Pour vérifier l'écriture du PID, on aurait pu également tester celui de vitesse car la mesure de vitesse était correct ce qui aurait pu isoler le problème de la mesure de courant. 

###Conclusion :
Nous avons réussit à controler un moteur Mcc par une commande de PWM complémentaire décalée. Pour cela, on a réussit à utiliser le shell à notre disposition en écrivant des fonctions liant commande shell et commande moteur. Nous avons su récupérer les composants (transistors, capteurs) à partir de la documentation et utilisait leurs datasheet et le cours à disposition pour mesurer des grandeurs du système (vitesse, courant).
A partir des simulations lors des Travaux dirigés, nous avons pu récupérer les coefficients du PID discret et implémenter un code permettant d'asservir le moteur. Il nous aura manqué un peu de temps pour debbug la mesure de courant et donc l'asservissement en boucle fermé du système. 




