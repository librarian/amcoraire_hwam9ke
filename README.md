# AmcorAire HW-AM9KE ESPHome Component

ESPHome external climate component for controlling an AmcorAire HW-AM9KE air conditioner over infrared.

## Usage

Add this repository as an external component in your ESPHome YAML:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/librarian/amcoraire_hwam9ke
      ref: main
    components: [amcoraire_hwam9ke]
```

Then add the climate platform:

```yaml
climate:
  - platform: amcoraire_hwam9ke
    name: "AmcorAire AC"
    id: amcoraire_ac
    pin: 4
```

The `pin` value is the GPIO number used for the IR transmitter. For example, `pin: 4` means GPIO4.

See [example.yaml](example.yaml) for a fuller ESP8266 configuration.

## Local Development

When changing the component locally, point ESPHome at the local `components` folder:

```yaml
external_components:
  - source:
      type: local
      path: components
    components: [amcoraire_hwam9ke]
```

Compile the minimal test config with Docker:

```
docker run --rm -v "${PWD}:/config" -w /config ghcr.io/esphome/esphome:stable compile tests/compile.yaml
```

Or, if ESPHome is installed locally:

```
esphome compile tests/compile.yaml
```
